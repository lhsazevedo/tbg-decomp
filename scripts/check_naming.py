#!/usr/bin/env python3
"""Check exported symbol names and public-function header coverage.

For every object under a plain `make` build (build/output/src/**/*.obj), inspects
its exported symbols via `sh4objtest inspect -x` and classifies each export as a
function (exported from section P, code) or data (exported from C/D/B). Since a
plain build compiles STATIC as `static`, every export here is already "public" by
construction.

Naming rules (see AGENTS.md "Naming Conventions"):
  - functions: a single camel/PascalCase word (no internal `_`) + `_8c<hex6>`,
    or the not-yet-named placeholder `FUN_8c<hex6>`. Only checked for objects
    compiled from a `.c` file -- functions still living in untouched
    `src/asm/*.src` keep whatever label the original disassembly gave them until
    someone actually decompiles that unit.
  - data: `var_`, `const_`, or `init_` prefix + word + `_8c<hex6>`, or the
    not-yet-named placeholder `var_8c<hex6>` / `const_8c<hex6>` / `init_8c<hex6>`,
    or `init_<Name>Segments` (route/course sub-arrays owned by a named parent
    struct, not directly addressed elsewhere). Data IS checked in `.src` objects
    too (the move-data skill renames data ownership independent of decompilation).
  - a small explicit allowlist of function names with no address suffix at all:
    SDK-mirrored names (`BupInit`, `main`, `scif_*`, ...) and hand-written
    infra/tooling functions (`serialprintf`) or SDK-header inline helpers that
    leak into a TU's exports (`intArrayChecksum`) -- none of these correspond to
    a decompiled game address, so a suffix would be meaningless.
  - `DEBUG_<Name>`: add-serial-logging skill's SERIAL_DEBUG-gated string tables,
    compile-time only, never referenced by address.

Also checks that every public function compiled from a `src/<unit>.c` file is
declared in `src/<unit>.h` (the unit's header must exist).

Also checks that no `src/<unit>.c` file has a local top-level `extern` declaration:
every external symbol must come from an `#include`d header instead (see AGENTS.md
"Key Conventions"), since C does not check an `extern` declaration against the
real definition across translation units -- a stale/wrong local `extern` silently
compiles and links as long as the name matches. Ownership of each externed symbol
is resolved the same way as everything else here: by inspecting which compiled
object actually exports that name (`sh4objtest inspect -x`, covering both a unit's
`.obj` from C and its `.obj.src` sibling from decompiled asm). A local extern for
a symbol the object's own `External relocations (imported symbols)` list never
references is flagged separately as dead code.

Usage: scripts/check_naming.py [build/output/src glob roots...]
Env:   SH4OBJTEST=path/to/sh4objtest   (default: sh4objtest)
"""
import os
import re
import subprocess
import sys
from pathlib import Path

SH4OBJTEST = os.environ.get("SH4OBJTEST", "sh4objtest")

IDX = re.compile(r"^Section (\d+): (\w)\s+\[")
EXPORT = re.compile(r"^\s{4}(\S+)\s+section=")
IMPORT = re.compile(r"^\s{4}addr=\S+\s+(\S+)\s+addend=")
EXTERN_DECL = re.compile(r"^extern\s+.*?\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\([^;]*\))?\s*(?:\[[^\]]*\])?\s*;\s*$")

FUNC_RE = re.compile(r"^[A-Za-z][A-Za-z0-9]*_8c[0-9a-f]{6}$")
DATA_RE = re.compile(r"^(?:var|const|init)_[A-Za-z][A-Za-z0-9]*_8c[0-9a-f]{6}$")
DATA_UNNAMED_RE = re.compile(r"^(?:var|const|init)_8c[0-9a-f]{6}$")
DATA_SEGMENTS_RE = re.compile(r"^init_[A-Za-z][A-Za-z0-9]*Segments$")
# add-serial-logging skill: SERIAL_DEBUG-gated string tables, no address (compile-time only)
DEBUG_NAMES_RE = re.compile(r"^DEBUG_[A-Za-z][A-Za-z0-9]*$")

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


def inspect_exports(obj):
    """Returns [(name_without_leading_underscore, kind)] for one object."""
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "-x"],
                         capture_output=True, text=True, check=True).stdout
    cur = None
    out = []
    for line in txt.splitlines():
        m = IDX.match(line)
        if m:
            cur = m.group(2)
            continue
        m = EXPORT.match(line)
        if m and cur is not None:
            name = m.group(1).lstrip("_")
            kind = "func" if cur == "P" else "data"
            out.append((name, kind))
    return out


def inspect_imports(obj):
    """Returns the set of symbol names (no leading underscore) an object's code
    actually references via external relocation -- ground truth for "is this
    extern used", independent of what's merely declared in the source."""
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "-x"],
                         capture_output=True, text=True, check=True).stdout
    out = set()
    for line in txt.splitlines():
        m = IMPORT.match(line)
        if m:
            out.add(m.group(1).lstrip("_"))
    return out


def owning_unit(obj, obj_root):
    """Returns (unit_stem, kind) for one object. kind is "c" for a unit's own
    compiled-from-C object, "asm_sibling" for that same unit's decompiled-asm
    stub (<unit>.obj.src), or "raw_asm" for an untouched, undecompiled unit
    (no .c counterpart at all, object lives under build/output/src/asm/)."""
    name = obj.name
    if name.endswith(".obj.src"):
        return name[:-len(".obj.src")], "asm_sibling"
    is_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
    return obj.stem, ("raw_asm" if is_raw_asm else "c")


TRAILING_COMMENT = re.compile(r"\s*//.*$")


def check_externs(repo_root, obj_root, units):
    """Flags local top-level `extern` declarations in `src/<unit>.c` files (see
    module docstring): each one should instead come from an #include'd header."""
    all_objs = sorted(obj_root.rglob("*.obj")) + sorted(obj_root.rglob("*.obj.src"))
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
        imported = inspect_imports(str(obj_path)) if obj_path.is_file() else set()

        for line in c_path.read_text(encoding="shift_jis").splitlines():
            stripped = TRAILING_COMMENT.sub("", line.strip())
            if not stripped.startswith("extern "):
                continue
            m = EXTERN_DECL.match(stripped)
            if not m:
                violations.append(f"[{unit}] unparsed extern line: {stripped!r}")
                continue
            name = m.group(1)
            if name in NO_HEADER_ALLOWLIST:
                continue
            bare = name.lstrip("_")
            owner = symbol_owner.get(bare)

            if owner is not None and owner[0] == unit and owner[1] == "c":
                # Defined later in this same .c file -- a same-object call needs
                # no external relocation, so this isn't a cross-unit extern at
                # all, just a forward declaration mistakenly written as one.
                violations.append(f"[{unit}] local extern '{name}' redeclares a "
                                   f"function/variable defined later in this same "
                                   f"file -- use a plain forward declaration "
                                   f"(no 'extern'), see AGENTS.md Forward Declarations")
                continue

            if bare not in imported:
                violations.append(f"[{unit}] local extern '{name}' is unused -- remove it")
            elif owner is None:
                violations.append(f"[{unit}] local extern '{name}' -- owning unit unknown")
            elif owner[0] == unit:
                violations.append(f"[{unit}] local extern '{name}' -- move to src/{unit}.h")
            elif owner[1] == "raw_asm":
                violations.append(f"[{unit}] local extern '{name}' -- owned by undecompiled "
                                   f"asm unit {owner[0]}, create src/{owner[0]}.h")
            else:
                violations.append(f"[{unit}] local extern '{name}' -- move to src/{owner[0]}.h")
    return violations


def check_name(name, kind):
    if kind == "func":
        if name in NO_ADDR_ALLOWLIST or FUNC_RE.match(name):
            return None
        return f"function '{name}' does not match Name_8c<addr> / FUN_8c<addr>"
    else:
        if (DATA_RE.match(name) or DATA_UNNAMED_RE.match(name)
                or DATA_SEGMENTS_RE.match(name) or DEBUG_NAMES_RE.match(name)):
            return None
        return f"data '{name}' does not match var_/const_/init_ Name_8c<addr>"


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
    objs = sorted(obj_root.rglob("*.obj")) if obj_root.is_dir() else []
    if not objs:
        sys.exit(f"No objects found under {obj_root} -- run `make` first")

    units = c_units((repo_root / "Makefile").read_text())
    header_cache = {}
    violations = []

    for obj in objs:
        # Objects under src/asm/ are compiled straight from untouched disassembly
        # (no .c counterpart) -- their function labels predate our naming
        # convention and aren't in scope until someone decompiles that unit.
        # Data IS still checked there: the move-data skill renames data ownership
        # independent of full decompilation.
        from_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
        for name, kind in inspect_exports(str(obj)):
            if kind == "func" and from_raw_asm:
                continue
            msg = check_name(name, kind)
            if msg:
                violations.append(f"[{obj.relative_to(repo_root)}] {msg}")

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

    if violations:
        print(f"{len(violations)} naming/header violation(s):\n")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)

    print(f"OK: {len(objs)} object(s) checked, no violations")


if __name__ == "__main__":
    main()
