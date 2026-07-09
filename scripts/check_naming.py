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
    "BupInit", "BupExit", "BupGetErrorString", "BupGetOperationString",
    "BupLoad", "BupSave", "BupDelete",
    "main",
    "scif_flush", "scif_getchar", "scif_init", "scif_isdata",
    "scif_putchar", "scif_puts",
    "serialprintf",
    "intArrayChecksum",
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

    if violations:
        print(f"{len(violations)} naming/header violation(s):\n")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)

    print(f"OK: {len(objs)} object(s) checked, no violations")


if __name__ == "__main__":
    main()
