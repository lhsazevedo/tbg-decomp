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

The plain build's call graph is necessarily incomplete mid-project: a symbol
whose only C-side caller lives in a not-yet-decompiled function looks private
there even though the shipped game calls it cross-unit. So a symbol is also
considered referenced if some OTHER unit's archived original `.src`
(build/output_matching/src/asm/decompiled/*.obj -- same source check_naming.py
trusts for original symbol names) still holds a live `.IMPORT` of it; that
`.IMPORT` is ground truth that the real game links it across units, whether or
not our decompilation has reached the call site yet.

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
BLOCK_COMMENT = re.compile(r"/\*.*?\*/", re.DOTALL)


def strip_comments(text):
    """Blank out /* ... */ and // comments (keeping newlines, so line-based
    scans and match positions elsewhere in the file are unaffected). A header
    prototype and a doc comment that happens to name that same function ahead
    of a "(" must not be conflated -- see header_declares."""
    text = BLOCK_COMMENT.sub(lambda m: "\n" * m.group().count("\n"), text)
    return "\n".join(TRAILING_COMMENT.sub("", line) for line in text.splitlines())


# Symbols we deliberately keep public even though nothing currently references
# them across units (so the object-level check would call them private):
#   - heap* :
#   - UnusedLoadInstall_8c014934 : unused
#   - ReplayMenuOpen_8c01673a : unused debug-menu entry point
KEEP_PUBLIC = {
    # unused general-purpose heap API
    "HeapInit_8c010fe8", "HeapAlloc_8c01102a", "HeapFree_8c0110c4",
    # unused
    "UnusedLoadInstall_8c014934",
    # unused debug-menu entry point
    "ReplayMenuOpen_8c01673a",
    # unreferenced global the original placed by declaring it in its header: SHC
    # lays B out by first declaration, so only a header declaration puts it
    # between var_cabinCamera_8c1bb984 and var_busState_8c1bb9d0
    "var_8c1bb9c4",
    # same: only a header declaration puts it ahead of 024b4c_bus_camera's
    # public vars
    "var_fixedCameraTarget_8c227d90",
    # same: only a header declaration puts it between 0289ac_objects' public
    # var_activePedPreset_8c22822c and var_pedGroupCount_8c228234
    "var_pedGroups_8c228230",
    # SHC changes the data layout otherwise
    "var_adxtHandles_8c0fcd20",
    # SHC changes the data layout otherwise
    "var_texlistQueueDoneCount_8c157a68",
    # SHC changes the data layout otherwise
    "var_datDir_8c18ad2c",
    # SHC changes the data layout otherwise
    "var_loadLatch_8c18adac",
    # SHC changes the data layout otherwise
    "var_routeModelIndexes_8c18adb0",
    # SHC changes the data layout otherwise
    "var_segmentHasStop_8c2286a4",
    # SHC changes the data layout otherwise
    "var_passengerActed_8c228958",
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
    # sh4objtest sometimes prints a "WARN: ..." diagnostic on stdout ahead
    # of the JSON doc itself (e.g. a debug-symbol chunk parse quirk) --
    # echo it so it stays visible, then skip straight to the object so it
    # doesn't break json.loads.
    brace = txt.index("{")
    if brace:
        print(txt[:brace], end="", file=sys.stderr)
    return json.loads(txt[brace:])


def owning_kind(obj, obj_root):
    """"c" for a unit's own compiled-from-C object, "raw_asm" for an
    undecompiled unit assembled straight from disassembly."""
    is_raw_asm = "asm" in obj.relative_to(obj_root).parts[:-1]
    return "raw_asm" if is_raw_asm else "c"


def header_declares(header_text, name, kind):
    """A function is declared by a prototype `... name(`; data by a bare
    word-boundary match on a non-comment line (mirrors check_naming.py). Both
    kinds are matched against comment-stripped text: a doc comment that merely
    *names* a function ahead of a following "(" (e.g. an unrelated call
    example) must not false-match as a declaration."""
    stripped = strip_comments(header_text)
    if kind == "func":
        return re.search(r"\b" + re.escape(name) + r"\s*\(", stripped) is not None
    word = re.compile(r"\b" + re.escape(name) + r"\b")
    for line in stripped.splitlines():
        if word.search(line):
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

    # A symbol's only C-side caller may live in a not-yet-decompiled function,
    # making it look private even though the shipped game links it cross-unit.
    # The archived original `.src` of every already-decompiled unit (same
    # source check_naming.py trusts) still holds the real `.IMPORT`s, so treat
    # a live one there as evidence the symbol is public. `.IMPORT` syntax can
    # only name a symbol the importing unit does NOT itself define, so these
    # entries are inherently cross-unit -- no self-reference to guard against.
    archived_root = repo_root / "build" / "output_matching" / "src" / "asm" / "decompiled"
    for obj in sorted(archived_root.glob("*.obj")) if archived_root.is_dir() else []:
        importer_id = f"archived:{obj.stem}"
        doc = inspect(str(obj))
        for sec in doc.get("sections", []):
            for r in sec.get("externalRelocations", []):
                importers.setdefault(r["name"].lstrip("_"), set()).add(importer_id)

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
