#!/usr/bin/env python3
"""Check that headers declare only a unit's *public* interface.

Complements check_naming.py, which enforces the forward half of the invariant
(every public function/extern is declared in its owning header; no `.c`
re-declares another unit's symbol). This enforces the reverse half: a header
must not declare a symbol that is *private* -- defined by a unit's own `.c` and
referenced by nothing outside that unit. Such a symbol belongs in the `.c`
(marked STATIC), not in any header.

Ground truth comes from the plain `make` build objects
(build/output/src/**/*.obj -- the C units' <unit>.obj and the undecompiled
asm units' asm/<name>.obj, i.e. exactly the link inputs minus the SDK libs).
For each linker symbol `sh4objtest inspect --format=json` gives the defining
object (sections[].exports[]) and every cross-object reference
(sections[].externalRelocations[]). A symbol is private iff no object other
than its definer references it.

A private symbol owned by a unit's `.c` is flagged only when its name is also
declared in some `src/*.h` -- that header declaration is the thing to remove.
Types/enums/macros/typedefs are never linker symbols, so they are inherently
ignored; data owned by an undecompiled asm unit is ignored too (no `.c` to
move it into).

Usage: scripts/check_private_decls.py
Env:   SH4OBJTEST=path/to/sh4objtest   (default: sh4objtest)
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

from check_naming import NO_ADDR_ALLOWLIST

SH4OBJTEST = os.environ.get("SH4OBJTEST", "sh4objtest")

TRAILING_COMMENT = re.compile(r"\s*//.*$")

# Functions we deliberately keep public even though nothing currently references
# them across units (so the object-level check would call them private):
#   - heap* : 
#   - FUN_8c014934 : unused
#   - DebugMenuOpen_8c01673a : unused debug-menu entry point
KEEP_PUBLIC = {
    # unused general-purpose heap API
    "HeapInit_8c010fe8", "HeapAlloc_8c01102a", "HeapFree_8c0110c4",
    # unused
    "FUN_8c014934",
    # unused debug-menu entry point
    "DebugMenuOpen_8c01673a",
}

# SDK-mirrored / infra names (scif_*, BupExit, main, serialprintf, ...) whose
# visibility is a contract, not usage-derived: they can look private in one build
# config yet be referenced by the SDK, boot code, or a serial-debug build. Reuse
# check_naming.py's set so the two checks agree. njUserInit/Main/Exit are not here
# -- they carry address suffixes and are already public via 010080_main.obj.
EXEMPT = NO_ADDR_ALLOWLIST | KEEP_PUBLIC


def inspect(obj):
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "--format=json"],
                         capture_output=True, text=True, check=True).stdout
    return json.loads(txt)


def owning_kind(obj, obj_root):
    """"c" for a unit's own compiled-from-C object, "raw_asm" for an
    undecompiled unit assembled straight from disassembly."""
    is_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
    return "raw_asm" if is_raw_asm else "c"


def header_declares(header_text, name, kind):
    """A function is declared by a prototype `... name(`; data by a bare
    word-boundary match on a non-comment line (mirrors check_naming.py)."""
    if kind == "func":
        return re.search(r"\b" + re.escape(name) + r"\s*\(", header_text) is not None
    word = re.compile(r"\b" + re.escape(name) + r"\b")
    for line in header_text.splitlines():
        if word.search(TRAILING_COMMENT.sub("", line)):
            return True
    return False


def main():
    repo_root = Path(__file__).resolve().parent.parent
    obj_root = repo_root / "build" / "output" / "src"
    objs = sorted(obj_root.rglob("*.obj")) if obj_root.is_dir() else []
    if not objs:
        sys.exit(f"No objects found under {obj_root} -- run `make` first")

    # owner[symbol] = (obj_id, owning_kind, symbol_kind); importers[symbol] = {obj_id}
    owner = {}
    importers = {}
    for obj in objs:
        obj_id = str(obj.relative_to(obj_root))
        okind = owning_kind(obj, obj_root)
        doc = inspect(str(obj))
        for sec in doc.get("sections", []):
            skind = "func" if sec.get("contents") == "code" else "data"
            for e in sec.get("exports", []):
                owner.setdefault(e["name"].lstrip("_"), (obj_id, okind, skind))
            for r in sec.get("externalRelocations", []):
                importers.setdefault(r["name"].lstrip("_"), set()).add(obj_id)

    headers = {p: p.read_text(encoding="shift_jis") for p in sorted((repo_root / "src").glob("*.h"))}

    violations = []
    for sym, (obj_id, okind, skind) in sorted(owner.items()):
        if okind != "c" or sym in EXEMPT:
            continue
        if importers.get(sym, set()) - {obj_id}:
            continue  # referenced elsewhere -> public
        for hp, text in headers.items():
            if header_declares(text, sym, skind):
                unit = obj_id[:-len(".obj")]
                violations.append(f"[{unit}] {skind} '{sym}' is private; make it "
                                  f"STATIC (NM_STATIC if the unit is in a matching "
                                  f"build) and remove from src/{hp.name}")
                break

    if violations:
        print(f"{len(violations)} private-declaration violation(s):\n")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)

    print(f"OK: {len(objs)} object(s) checked, no private declarations in headers")


if __name__ == "__main__":
    main()
