#!/usr/bin/env python3
"""Compare each decompiled unit's imported symbol set against its archived
assembly.

scripts/dcdiff.py compares the data sections (C/D/B) byte for byte, but never
looks at section P, so a unit can be data-perfect and still reference the wrong
symbols. This check closes that gap from the other side: the set of names a unit
imports and exports is invariant to register allocation, scheduling and
instruction selection, so it can be compared directly even though the build is
not byte-matching.

The archived `.src` is the reference (not the matching-build object) so that the
units the matching build compiles from C are covered too -- those have no asm
object, and are exactly the ones nothing else checks.

Three classes of difference are noise and are normalised away:

  * runtime helpers carry a varying number of leading underscores (`__divls` vs
    `divls`), so all leading underscores are stripped on both sides;
  * the serial-logging symbols only exist in a SERIAL_DEBUG build, so the caller
    must build without it (the default);
  * the project links its own string implementations under different names.

A divergence means one side references something the other does not -- usually a
call left out of the C, or a data layout split differently from the original.
NOT_CHECKED lists the units where the difference is understood and unfixed;
everything else fails the build.

Exports are deliberately not compared. They cannot hide a bug -- a symbol some
unit imports must be exported by whatever defines it, or the link fails, so the
linker already enforces that half. They also differ structurally for reasons
that are artifacts of the archive rather than facts about the game: it names
every string literal (`const_8c0360a4`), because disassembling a linked image
requires a symbol per referenced address, while the C keeps them as literals in
one section C blob. Imports are the half that fails open.
"""
import json
import os
import re
import subprocess
import sys

# Units with a known, understood divergence. Drop a unit once it is resolved.
NOT_CHECKED = {
    # startReplaySave_8c016924 calls the LZW packer; nothing in the image calls
    # startReplaySave, so the C is unreachable and under-decompiled.
    "01614c_replay_menu": {"ReplayCodecPack_8c02f934"},
    # The archive's out-of-memory hook and its division are not in the C yet.
    "010fe8_heap": {"divls", "heapAllocOutOfMemoryHook_8c01102a"},
    # The archive streams from GD-ROM and stops MIDI; the C does neither yet.
    "0100bc_sound": {"gdFsClose", "gdFsGetFileSize", "gdFsOpen", "gdFsRead",
                     "sdMidiStopAll"},
    # unpackGlyph_8c015110 clears with a runtime loop; the archive memcpys a
    # zeroed section C template (see docs/lessons_learned.md).
    "015034_text": {"slow_mvn"},
    # The archive calls the SDK distance helper; the C computes it inline.
    "020594_vehicle_model": {"njDistanceP2P"},
}

ALIASES = {"slow_strcpy": "strcpy", "slow_strcmp1": "strcmp",
           "slow_strlen": "strlen"}


def norm(name):
    return ALIASES.get(name.lstrip("_"), name.lstrip("_"))


def archive_imports(path):
    names = set()
    for line in open(path, "rb").read().decode("latin-1").splitlines():
        m = re.match(r"\.IMPORT\s+(\S+)", line.strip(), re.I)
        if m:
            names.add(norm(m.group(1)))
    return names


def object_imports(path):
    out = subprocess.run(["sh4objtest", "inspect", "--format=json", path],
                         capture_output=True, text=True)
    if out.returncode != 0:
        raise SystemExit("sh4objtest inspect failed on %s:\n%s" % (path, out.stderr))
    data = json.loads(out.stdout)
    names = set()
    for sec in data["sections"]:
        for rel in sec.get("externalRelocations") or []:
            name = rel.get("name") or rel.get("symbol")
            if name:
                names.add(norm(name))
    return names


def main():
    fail = 0
    checked = 0
    for src in sorted(os.listdir("src/asm/decompiled")):
        if not src.endswith(".src"):
            continue
        unit = src[:-4]
        obj = "build/output/src/%s.obj" % unit
        if not os.path.exists(obj):
            continue
        checked += 1
        archive = archive_imports(os.path.join("src/asm/decompiled", src))
        c = object_imports(obj)
        allowed = NOT_CHECKED.get(unit, set())
        only_a, only_c = archive - c - allowed, c - archive - allowed
        diffs = []
        if only_a:
            diffs.append("  archive imports, C does not: %s"
                         % ", ".join(sorted(only_a)))
        if only_c:
            diffs.append("  C imports, archive does not: %s"
                         % ", ".join(sorted(only_c)))
        if diffs:
            print("=== %s ===" % unit)
            print("\n".join(diffs))
            fail = 1
    if fail:
        print("Some units do not match the archive's import set")
    else:
        print("All %d checked units match the archive's import set" % checked)
    return fail


if __name__ == "__main__":
    sys.exit(main())
