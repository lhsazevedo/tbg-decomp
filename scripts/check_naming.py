#!/usr/bin/env python3
"""Check symbol names, the public/private naming split, and header coverage.

Symbols are read from the built objects (not source) via `sh4objtest inspect`,
covering three object kinds so a name is checked wherever it is defined:
  - a decompiled unit's C object (build/output/src/<unit>.obj),
  - that unit's archived `.src` assembled by the matching build
    (build/output_matching/src/asm/decompiled/<unit>.obj) -- this is where a
    STATIC private, or a symbol SHC inlined away, still has a real label,
  - an undecompiled raw-asm unit's object (build/output/src/asm/**/*.obj), where
    only exported symbols are in scope (the rest are `FUN_`/local until named).
`inspect --format=json` `debugSymbols` gives every defined symbol and, via its
`externalName`, whether it is exported -- i.e. public in the real program (a
plain build keeps STATIC private; a private only shows as exported under
UNIT_TESTING or a stray unconditional `.EXPORT`).

Naming rules (see AGENTS.md "Naming Conventions"):
  - public (exported) functions: `<ShortUnit><Name>_8c<hex6>`, PascalCase, where
    <ShortUnit> is the unit's PascalCase token declared with an `@unit` comment
    in its primary source (`/* @unit Pause */` in the `.c`, or `; @unit ...` in
    the `.src` of a still-assembly unit).
  - private (STATIC) functions: `<name>_8c<hex6>`, camelCase, lowercase first
    letter, no internal `_`.
  - either, not-yet-named: `FUN_8c<hex6>` or partially named
    `<ShortUnit>FUN_8c<hex6>` (e.g. CourseMenuFUN_8c017ef2).
  - public (exported) data: `var_`/`const_`/`init_` + word + `_8c<hex6>`, or the
    unnamed placeholder `{var,const,init}_8c<hex6>`, or `init_<Name>Segments`.
    Private data is out of scope: it includes address-less sub-arrays owned by a
    parent struct (the course PacFiles lists), addressed only through the parent.
  - a small allowlist of names with no address suffix (SDK-mirrored `main`,
    `scif_*`, ...; infra `serialprintf`; SDK inline `intArrayChecksum`) and
    `DEBUG_<Name>` (SERIAL_DEBUG string tables, compile-time only).

Cross-checks the C object and archived `.src` object of each decompiled unit: a
rename keeps the `_8c<addr>` suffix, so at any address defined in both, the names
must agree (symbols present in only one -- SHC inlined a static, compiler data
layout -- are not drift).

Also checks that every public function compiled from a `src/<unit>.c` file is
declared in `src/<unit>.h` (the unit's header must exist).

Also checks that no `src/<unit>.c` file has a local top-level declaration of a
symbol defined elsewhere: every external symbol must come from an `#include`d
header instead (see AGENTS.md "Key Conventions"), since C does not check a
declaration against the real definition across translation units -- a
stale/wrong local one silently compiles and links as long as the name
matches. Ground truth is `sh4objtest inspect --format=json`'s `debugSymbols`:
a file-scope `Var`/`Func` entry with `fileNumber == 0` (the unit's own `.c`
file) and no `section`/`address` (no definition present in this object) is a
bare declaration, independent of whether the literal `extern` keyword was
used. Ownership of each such symbol is resolved the same way as everything
else here: by inspecting which compiled object actually exports that name
(`sh4objtest inspect --format=json`'s `sections[].exports`, covering both a
unit's `.obj` from C and its `.obj.src` sibling from decompiled asm). A
declaration for a symbol the object's own `sections[].externalRelocations`
list never references is flagged separately as dead code -- note SHC omits an
unreferenced extern from debug info entirely, so this dead-code check can only
rely on the relocation list, not `debugSymbols`.

Usage: scripts/check_naming.py [build/output/src glob roots...]
Env:   SH4OBJTEST=path/to/sh4objtest   (default: sh4objtest)
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

SH4OBJTEST = os.environ.get("SH4OBJTEST", "sh4objtest")

EXTERN_DECL = re.compile(r"^extern\s+.*?\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\([^;]*\))?\s*(?:\[[^\]]*\])?\s*;\s*$")
TRAILING_COMMENT = re.compile(r"\s*//.*$")

# Public functions carry the unit's PascalCase ShortUnit prefix + a PascalCase
# name (built per-unit below from the `@unit` tag). Private functions are a single
# camelCase word, lowercase first letter, no internal `_`. Both end in `_8c<hex6>`.
FUNC_PRIVATE_RE = re.compile(r"^[a-z][A-Za-z0-9]*_8c[0-9a-f]{6}$")
# not-yet-named originals: `FUN_8c<addr>`, or partially named `<ShortUnit>FUN_8c<addr>`
# (e.g. CourseMenuFUN_8c017ef2) -- exempt for both public and private.
FUNC_EXEMPT_RE = re.compile(r"^(?:[A-Z][A-Za-z0-9]*)?FUN_8c[0-9a-f]{6}$")
DATA_RE = re.compile(r"^(?:var|const|init)_[A-Za-z][A-Za-z0-9]*_8c[0-9a-f]{6}$")
DATA_UNNAMED_RE = re.compile(r"^(?:var|const|init)_8c[0-9a-f]{6}$")
DATA_SEGMENTS_RE = re.compile(r"^init_[A-Za-z][A-Za-z0-9]*Segments$")
# add-serial-logging skill: SERIAL_DEBUG-gated string tables, no address (compile-time only)
DEBUG_NAMES_RE = re.compile(r"^DEBUG_[A-Za-z][A-Za-z0-9]*$")

# `/* @unit Pause */` (in a .c) or `; @unit Pause` (in a .src): the unit's ShortUnit.
UNIT_TAG_RE = re.compile(r"@unit\s+(\S+)")
SHORTUNIT_RE = re.compile(r"^[A-Z][A-Za-z0-9]*$")
ADDR_SUFFIX_RE = re.compile(r"_8c([0-9a-f]{6})$")


def public_func_re(shortunit):
    return re.compile(r"^" + re.escape(shortunit) + r"[A-Z][A-Za-z0-9]*_8c[0-9a-f]{6}$")

# functions with no address suffix: SDK-mirrored names (compiler/linker enforce these
# are correct) plus hand-written infra/tooling and SDK-header inline helpers, neither
# of which corresponds to a decompiled game address.
NO_ADDR_ALLOWLIST = {
    "BupExit",
    "main",
    "scif_flush", "scif_getchar", "scif_init", "scif_isdata",
    "scif_putchar", "scif_puts",
    "serialprintf",
    "intArrayChecksum",
}

# symbols a local `extern` may legitimately name with no header available: defined
# by the linker command file (build/lnk_matching*.sub), not any project unit.
NO_HEADER_ALLOWLIST = {
    "_BSG_END",
}


# Raw `sh4objtest inspect --format=json` output, cached per object so each
# object is only ever actually invoked once even though several unrelated call
# sites (the main scan, the header check, check_externs, ...) all want to look
# at it. A single JSON doc covers everything every inspect_* function below
# needs -- exports, external relocations, and debug symbols alike -- so there
# is no separate "-x" text-format cache to keep in sync with this one.
_JSON_CACHE = {}   # obj -> parsed doc of `inspect --format=json`

# Progress ticker: each first-time `inspect` call is the checker's real cost,
# so count against the full object set up front and print "n/total" as they
# happen, overwriting in place. Silent when stderr isn't a terminal (CI logs).
_progress = {"done": 0, "total": 0}


def _progress_tick(obj):
    if not _progress["total"] or not sys.stderr.isatty():
        return
    _progress["done"] += 1
    print(f"\rchecking symbols... {_progress['done']}/{_progress['total']} "
          f"({Path(obj).name})" + " " * 10, end="", file=sys.stderr)


def _inspect_json(obj):
    if obj not in _JSON_CACHE:
        txt = subprocess.run([SH4OBJTEST, "inspect", obj, "--format=json"],
                              capture_output=True, text=True, check=True).stdout
        _JSON_CACHE[obj] = json.loads(txt)
        _progress_tick(obj)
    return _JSON_CACHE[obj]


def inspect_exports(obj):
    """Returns [(name_without_leading_underscore, kind)] for one object."""
    doc = _inspect_json(obj)
    out = []
    for sec in doc.get("sections", []):
        kind = "func" if sec.get("contents") == "code" else "data"
        for e in sec.get("exports", []):
            out.append((e["name"].lstrip("_"), kind))
    return out


def inspect_imports(obj):
    """Returns the set of symbol names (no leading underscore) an object's code
    actually references via external relocation -- ground truth for "is this
    extern used", independent of what's merely declared in the source."""
    doc = _inspect_json(obj)
    out = set()
    for sec in doc.get("sections", []):
        for r in sec.get("externalRelocations", []):
            out.add(r["name"].lstrip("_"))
    return out


def owning_unit(obj, obj_root):
    """Returns (unit_stem, kind) for one object. kind is "c" for a unit's own
    compiled-from-C object, or "raw_asm" for an untouched, undecompiled unit
    (no .c counterpart at all, object lives under build/output/src/asm/)."""
    is_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
    return obj.stem, ("raw_asm" if is_raw_asm else "c")


def inspect_bare_declarations(obj):
    """Returns the set of file-scope Var/Func names that a unit's own `.c` file
    (debugSymbols fileNumber == 0, i.e. sourceFiles[0]) declares but never
    defines in this object (section/address both None). This is ground truth
    for "declared locally with no local definition" regardless of whether the
    declaration used the `extern` keyword -- a bare `void Foo(int);` with no
    `extern` is just as much a violation as `extern void Foo(int);`, and the
    old text-regex approach could only ever see the latter."""
    doc = _inspect_json(obj)
    out = set()
    for s in doc.get("debugSymbols", []):
        if (s["nesting"] == 0 and s["type"] in ("Var", "Func")
                and s["fileNumber"] == 0 and s["section"] is None):
            out.add(s["name"])
    return out


def inspect_defined(obj):
    """Returns [(bare_name, kind, is_public)] for every symbol DEFINED in this
    object, handling both compiler outputs:
      - shc (C): `debugSymbols` Func/Var with fileNumber == 0 and a section.
        `externalName` is set exactly when the symbol is exported (public); a
        STATIC symbol has it None.
      - asmsh (.src, -debug): `debugSymbols` Label whose name starts with `_`
        (a real C-ABI symbol, excluding local `LAB_`/anonymous labels) and has a
        section. `externalName` set == `.EXPORT`ed. code/data comes from the
        symbol's section index into `sections[].contents`.
    is_public is the object's own export state, so it reflects the real program:
    a plain build leaves STATIC private (not exported); a private symbol only
    surfaces as public in an object built with UNIT_TESTING (or a stray
    unconditional `.EXPORT` in a decompiled `.src`)."""
    doc = _inspect_json(obj)
    sec_kind = ["func" if s.get("contents") == "code" else "data"
                for s in doc.get("sections", [])]
    out = []
    for s in doc.get("debugSymbols", []):
        t, sect, name = s["type"], s["section"], s["name"]
        if t in ("Var", "Func"):
            if s["fileNumber"] != 0 or sect is None:
                continue
            out.append((name, "func" if t == "Func" else "data",
                        s["externalName"] is not None))
        elif t == "Label":
            if not name.startswith("_") or sect is None:
                continue
            kind = sec_kind[sect] if 0 <= sect < len(sec_kind) else "data"
            out.append((name[1:], kind, s["externalName"] is not None))
    return out


def unit_shortunit(repo_root, unit_stem):
    """Locate a unit's primary source file (its `.c`, else its undecompiled
    `.src`) and return (source_rel_path_str_or_None, shortunit_or_None,
    error_or_None). error is set when the `@unit` token is present but malformed."""
    src = repo_root / "src" / f"{unit_stem}.c"
    if not src.is_file():
        cands = sorted((repo_root / "src" / "asm").rglob(f"{unit_stem}.src"))
        src = cands[0] if cands else None
    if src is None:
        return None, None, None
    m = UNIT_TAG_RE.search(src.read_text(encoding="shift_jis"))
    rel = str(src.relative_to(repo_root))
    if not m:
        return rel, None, None
    token = m.group(1)
    if not SHORTUNIT_RE.match(token):
        return rel, None, f"{rel}: `@unit {token}` is not a PascalCase token"
    return rel, token, None


def check_externs(repo_root, obj_root, units):
    """Flags local top-level declarations of externally-defined symbols in
    `src/<unit>.c` files (see module docstring): each one should instead come
    from an #include'd header. Also flags the literal `extern` keyword used on
    a same-file forward declaration -- misleading style, since no cross-unit
    reference is actually involved there."""
    # `*.obj.src` (an asm-listing dump from a second `shc -code=asm` pass, used
    # by scripts/bnc.sh, not by this checker) is text, not an object -- skip it.
    all_objs = sorted(obj_root.rglob("*.obj"))
    symbol_owner = {}
    for obj in all_objs:
        stem, kind = owning_unit(obj, obj_root)
        for name, _kind in inspect_exports(str(obj)):
            symbol_owner.setdefault(name, (stem, kind))

    violations = []
    for unit in units:
        c_path = repo_root / "src" / f"{unit}.c"
        if not c_path.is_file():
            continue
        obj_path = obj_root / f"{unit}.obj"
        if not obj_path.is_file():
            continue
        imported = inspect_imports(str(obj_path))
        bare_declared = inspect_bare_declarations(str(obj_path))

        extern_named = set()
        for line in c_path.read_text(encoding="shift_jis").splitlines():
            stripped = TRAILING_COMMENT.sub("", line.strip())
            if not stripped.startswith("extern "):
                continue
            m = EXTERN_DECL.match(stripped)
            if not m:
                violations.append(f"[{unit}] unparsed extern line: {stripped!r}")
                continue
            extern_named.add(m.group(1))

        for name in sorted(extern_named | bare_declared):
            if name in NO_HEADER_ALLOWLIST:
                continue
            bare = name.lstrip("_")
            owner = symbol_owner.get(bare)
            has_extern_kw = name in extern_named
            label = f"local extern '{name}'" if has_extern_kw else f"local declaration of '{name}'"

            if owner is not None and owner[0] == unit and owner[1] == "c":
                # Defined later in this same .c file -- a same-object call needs
                # no external relocation, so this isn't a cross-unit extern at
                # all. A plain forward declaration (no 'extern') is fine and
                # won't show up here at all; only the 'extern'-keyword style is
                # flagged, since it's misleading about where the symbol lives.
                if has_extern_kw:
                    violations.append(f"[{unit}] {label} redeclares a "
                                       f"function/variable defined later in this same "
                                       f"file -- use a plain forward declaration "
                                       f"(no 'extern'), see AGENTS.md Forward Declarations")
                continue

            if bare not in imported:
                # SHC drops an unreferenced extern from debug info entirely, so
                # a truly dead declaration never shows up in bare_declared --
                # `imported` (external relocations, independent of debug info)
                # is the only reliable signal for "is this actually used".
                violations.append(f"[{unit}] {label} is unused -- remove it")
            elif owner is None:
                violations.append(f"[{unit}] {label} -- owning unit unknown")
            elif owner[0] == unit:
                violations.append(f"[{unit}] {label} -- move to src/{unit}.h")
            elif owner[1] == "raw_asm":
                violations.append(f"[{unit}] {label} -- owned by undecompiled "
                                   f"asm unit {owner[0]}, create src/{owner[0]}.h")
            else:
                violations.append(f"[{unit}] {label} -- move to src/{owner[0]}.h")
    return violations


def check_symbol(name, kind, is_public, shortunit):
    """Returns a violation message for one defined symbol, or None if it conforms.
    `shortunit` is the owning unit's ShortUnit (or None if untagged)."""
    if kind == "data":
        # Only public (exported) data is in scope. A unit's private data includes
        # intentionally address-less sub-arrays owned by a parent struct (e.g.
        # init_<Name>Segments and the route/course PacFiles lists), addressed only
        # through the parent -- not a naming concern of their own.
        if not is_public:
            return None
        if (DATA_RE.match(name) or DATA_UNNAMED_RE.match(name)
                or DATA_SEGMENTS_RE.match(name) or DEBUG_NAMES_RE.match(name)):
            return None
        return f"data '{name}' does not match var_/const_/init_<Name>_8c<addr>"
    # func
    if name in NO_ADDR_ALLOWLIST or FUNC_EXEMPT_RE.match(name):
        return None
    if is_public:
        if shortunit is None:
            return (f"public function '{name}' but its unit has no "
                    f"`@unit <ShortUnit>` tag")
        if public_func_re(shortunit).match(name):
            return None
        return (f"public function '{name}' must be "
                f"{shortunit}<PascalCase>_8c<addr>")
    if FUNC_PRIVATE_RE.match(name):
        return None
    return (f"private function '{name}' must be camelCase <name>_8c<addr> "
            f"(lowercase first letter, no internal '_')")


def c_units(makefile):
    srcs = re.search(r"^SRCS\s*=(.*?)^\S", makefile, re.S | re.M)
    units = []
    for line in srcs.group(1).splitlines():
        line = line.strip().rstrip("\\").strip()
        if line.startswith("src/") and line.endswith(".c") and "/asm/" not in line:
            units.append(Path(line).stem)
    return units


def header_declares(header_text, name):
    return re.search(r"\b" + re.escape(name) + r"\s*\(", header_text) is not None


def main():
    repo_root = Path(__file__).resolve().parent.parent
    obj_root = repo_root / "build" / "output" / "src"
    # Archived `.src` of decompiled units, assembled by the matching build. This is
    # where a unit's original disassembly symbols live (incl. STATIC privates and
    # SHC-inlined ones that never get a standalone symbol in the C object), so we
    # check symbol names here too, not just in the C object.
    src_root = repo_root / "build" / "output_matching" / "src" / "asm" / "decompiled"
    objs = sorted(obj_root.rglob("*.obj")) if obj_root.is_dir() else []
    if not objs:
        sys.exit(f"No objects found under {obj_root} -- run `make` first")

    units = c_units((repo_root / "Makefile").read_text())
    header_cache = {}
    violations = []
    naming_v = set()  # symbol/drift/tag violations, deduped across the .c and .src objects

    su_cache = {}
    def shortunit(unit_stem):
        if unit_stem not in su_cache:
            _rel, su, err = unit_shortunit(repo_root, unit_stem)
            if err:
                naming_v.add(f"[{unit_stem}] {err}")
            su_cache[unit_stem] = su
        return su_cache[unit_stem]

    # Naming rules, applied per defined symbol of every object:
    #  - decompiled C object + its archived `.src` object: full defined set (the
    #    object's own export state gives each symbol's public/private status).
    #  - undecompiled raw-asm object (src/asm/**, no .c): only exported symbols --
    #    original disassembly is all `FUN_`/unnamed until someone names a symbol
    #    "by usage", and its private internal labels aren't in scope yet.
    # deduped: a symbol wrong the same way in both the .c and .src objects (e.g. a
    # private in both) reports once.
    def scan(obj, symbols):
        su = shortunit(obj.stem)
        for name, kind, is_public in symbols:
            msg = check_symbol(name, kind, is_public, su)
            if msg:
                naming_v.add(f"[{obj.stem}] {msg}")

    src_objs = sorted(src_root.glob("*.obj")) if src_root.is_dir() else []
    _progress["total"] = len(objs) + len(src_objs)

    for obj in objs:
        from_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
        if from_raw_asm:
            # Undecompiled unit: its exports are the original disassembly's, mostly
            # `FUN_`/unnamed. Enforce function names only once the unit opts in with
            # an `@unit` tag (in its .src) -- a name given "by usage" before that is
            # tolerated, like FUN_. Data is always checked.
            tagged = shortunit(obj.stem) is not None
            scan(obj, [(n, k, True) for n, k in inspect_exports(str(obj))
                       if tagged or k != "func"])
        else:
            scan(obj, inspect_defined(str(obj)))
    for obj in src_objs:
        scan(obj, inspect_defined(str(obj)))

    # Cross-check the C object and the archived `.src` object of each decompiled
    # unit: a rename keeps the `_8c<addr>` suffix, so at any address defined in
    # both, the names must agree. Symbols present in only one object (SHC inlined a
    # static; compiler-specific data layout) are skipped -- not drift.
    for src_obj in src_objs:
        c_obj = obj_root / f"{src_obj.stem}.obj"
        if not c_obj.is_file():
            continue
        def by_addr(o):
            d = {}
            for n, _k, _p in inspect_defined(str(o)):
                m = ADDR_SUFFIX_RE.search(n)
                if m:
                    d[m.group(1)] = n
            return d
        c_map, s_map = by_addr(c_obj), by_addr(src_obj)
        for addr in sorted(c_map.keys() & s_map.keys()):
            if c_map[addr] != s_map[addr]:
                naming_v.add(f"[{src_obj.stem}] symbol at 8c{addr} differs: "
                             f".c has '{c_map[addr]}', .src has '{s_map[addr]}' "
                             f"-- rename both")

    for unit in units:
        header_path = repo_root / "src" / f"{unit}.h"
        obj_path = obj_root / f"{unit}.obj"
        if not obj_path.is_file():
            continue
        funcs = [n for n, k in inspect_exports(str(obj_path))
                 if k == "func" and n not in NO_ADDR_ALLOWLIST]
        if not funcs:
            continue
        if not header_path.is_file():
            violations.append(f"[{unit}] has public functions but no src/{unit}.h")
            continue
        header_text = header_cache.setdefault(header_path, header_path.read_text(encoding="shift_jis"))
        for name in funcs:
            if not header_declares(header_text, name):
                violations.append(f"[{unit}] public function '{name}' not declared in src/{unit}.h")

    violations.extend(check_externs(repo_root, obj_root, units))
    violations = sorted(naming_v) + violations

    if _progress["total"] and sys.stderr.isatty():
        print("\r" + " " * 60 + "\r", end="", file=sys.stderr)

    if violations:
        print(f"{len(violations)} naming/header violation(s):\n")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)

    print(f"OK: {len(objs)} object(s) checked, no violations")


if __name__ == "__main__":
    main()
