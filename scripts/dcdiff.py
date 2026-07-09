#!/usr/bin/env python3
"""Diff two SH4 objects' data sections via `sh4objtest inspect -x`.

Goal is functional equivalence, so section P (code, incl. literal pools) may differ;
C (const), D (init data) and B (bss) should match when the objects are meant to be
equivalent. Two things are normalised so faithful pointers don't look like diffs:
  * relocated 4-byte slots are masked out before the byte compare (the assembler
    stores addends in the reloc entry, the compiler stores them in-place);
  * relocations are compared as an ordered list of TARGETS (external symbol, or
    internal section letter), ignoring the addend.

Best for objects that should be identical: before/after a rename, or isolated
data-only objects. A not-yet-matched unit whose compiler lays constants out in a
different order will (correctly) report a difference.

Usage: scripts/dcdiff.py <a.obj> <b.obj>
Env:   SH4OBJTEST=path/to/sh4objtest   (default: sh4objtest)
"""
import os, re, subprocess, sys

SH4OBJTEST = os.environ.get("SH4OBJTEST", "sh4objtest")
SEC = re.compile(r"^Section \d+: (\w)\s+\[")
IDX = re.compile(r"^Section (\d+): (\w)\s+\[")
HEX = re.compile(r"^\s*0x([0-9a-fA-F]+):\s+(.*)$")
INT = re.compile(r"addr=0x([0-9a-fA-F]+)\s+-> section (\d+)\s+addend=(\S+)")
EXT = re.compile(r"addr=0x([0-9a-fA-F]+)\s+(\S+)\s+addend=0x[0-9a-fA-F]+\s+width=")


def inspect(obj):
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "-x"],
                         capture_output=True, text=True, check=True).stdout
    idx2letter, raw, cur = {}, {}, None
    for line in txt.splitlines():
        m = IDX.match(line)
        if m:
            cur = m.group(2)
            idx2letter[m.group(1)] = cur
            raw.setdefault(cur, {"bytes": {}, "rel": []})
            continue
        if cur is None:
            continue
        m = HEX.match(line)
        if m:
            off = int(m.group(1), 16)
            for i, b in enumerate(m.group(2).split("|")[0].split()):
                raw[cur]["bytes"][off + i] = int(b, 16)
            continue
        m = INT.search(line)
        if m:
            raw[cur]["rel"].append((int(m.group(1), 16), ("S", m.group(2))))
            continue
        m = EXT.search(line)
        if m:
            raw[cur]["rel"].append((int(m.group(1), 16), ("X", m.group(2))))
    secs = {}
    for letter, r in raw.items():
        size = (max(r["bytes"]) + 1) if r["bytes"] else 0
        buf = bytearray(size)
        for off, b in r["bytes"].items():
            buf[off] = b
        offsets = sorted(o for o, _ in r["rel"])
        for o in offsets:
            buf[o:o + 4] = b"\0\0\0\0"                      # mask reloc slots
        targets = [(k, idx2letter.get(v, v) if k == "S" else v)
                   for _, (k, v) in sorted(r["rel"])]        # ordered, addend dropped
        secs[letter] = (buf, targets)
    return secs


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    a, b = inspect(sys.argv[1]), inspect(sys.argv[2])
    diffs = 0
    for letter in sorted(set(a) | set(b)):
        tag = "code, differences expected" if letter == "P" else None
        if letter not in a or letter not in b:
            print(f"[{letter}] only in {sys.argv[1] if letter in a else sys.argv[2]}")
            diffs += letter != "P"
            continue
        (ba, ta), (bb, tb) = a[letter], b[letter]
        msgs = []
        if ba != bb:
            n = min(len(ba), len(bb))
            off = next((i for i in range(n) if ba[i] != bb[i]), n)
            msgs.append(f"data differs at 0x{off:x} (len {len(ba)} vs {len(bb)})")
        if ta != tb:
            n = min(len(ta), len(tb))
            i = next((k for k in range(n) if ta[k] != tb[k]), n)
            first = f"{ta[i] if i < len(ta) else None} vs {tb[i] if i < len(tb) else None}"
            msgs.append(f"{sum(x != y for x, y in zip(ta, tb)) + abs(len(ta) - len(tb))} "
                        f"reloc target(s) differ; first at #{i}: {first}")
        if msgs:
            print(f"[{letter}] " + "; ".join(msgs) + (f"  ({tag})" if tag else ""))
            diffs += letter != "P" and bool(msgs)
        else:
            print(f"[{letter}] match ({len(ba)} bytes, {len(ta)} relocs)")
    print("\nMATCH" if diffs == 0 else f"\n{diffs} section(s) differ")
    sys.exit(1 if diffs else 0)


if __name__ == "__main__":
    main()
