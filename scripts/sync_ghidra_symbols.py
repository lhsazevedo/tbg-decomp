#!/usr/bin/env python3
"""Sync symbol names from the matching build into the Ghidra project
(flycast_ram.bin). Re-runnable; build always wins.

Prereqs: the matching build must be up to date (`make -f Makefile.matching`) and
assembled with `-debug` (embeds debug symbols in the .obj -- required for statics,
see docs/lessons_learned.md), and Ghidra must be open with the GhidraMCP plugin
listening on GHIDRA_URL.

Default run prints a summary only; pass --apply to write to Ghidra.

Two symbol sources, unified:
  1. `sh4objtest inspect --format=json` over every build/output_matching/*.obj
     -- exports (public) + debug symbols (Func/Var from C objs, Label from asm
     objs, the latter only present because the matching build now assembles with
     `-debug`, which embeds debug info in the .obj instead of a side .DWF).
  2. build/lnk_matching_template.sub `define _NAME(8CADDR)` -- SDK funcs (external
     refs, absent from the object exports).

Address resolution is map-free / self-anchoring: nearly every symbol name encodes
its absolute address (`_foo_8c013ae8`), and inspect gives each symbol's
section-relative offset, so base = addr - offset per (object, section). That base
then places the few address-less-named symbols (init_courseOmeDaySegments, ...).

Apply goes to the live GhidraMCP plugin over HTTP (GUI stays open); build wins.
Functions -> /rename_function_by_address (strict_mode off; /create_function
fallback). Data -> /batch_create_labels.
"""

import json
import re
import shutil
import subprocess
import sys
import urllib.request
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent   # scripts/ -> repo root
OBJ_DIR = REPO_ROOT / "build" / "output_matching"
SUB_FILE = REPO_ROOT / "build" / "lnk_matching_template.sub"
DUMP_DIR = REPO_ROOT / "build" / "inspect_matching_tmp"

GHIDRA_URL = "http://127.0.0.1:8089"
PROGRAM = "flycast_ram.bin"

APPLY = "--apply" in sys.argv          # default is a dry run (print summary only)

ADDR_RE = re.compile(r"_(8[cC][0-9a-fA-F]{6})$")


# ---------------------------------------------------------------------------
# Extraction
# ---------------------------------------------------------------------------

def dump_inspect_json():
    """Run sh4objtest inspect on every matching object inside Docker (one shot),
    writing per-object JSON to DUMP_DIR. Returns list of (obj_path, json_path)."""
    objs = sorted(str(p.relative_to(REPO_ROOT)) for p in OBJ_DIR.rglob("*.obj"))
    DUMP_DIR.mkdir(parents=True, exist_ok=True)
    pairs = []
    lines = ["set -e", "mkdir -p build/inspect_matching_tmp"]
    for rel in objs:
        flat = rel.replace("/", "_")
        out = f"build/inspect_matching_tmp/{flat}.json"
        lines.append(f'sh4objtest inspect "{rel}" --format=json -o "{out}"')
        pairs.append((rel, REPO_ROOT / out))
    script = "\n".join(lines)
    print(f"[*] inspecting {len(objs)} objects in one Docker run ...")
    subprocess.run(["./docker-run.sh", "bash", "-c", script],
                   cwd=REPO_ROOT, check=True)
    return pairs


def parse_addr(name):
    m = ADDR_RE.search(name)
    return int(m.group(1), 16) if m else None


def candidates_from_object(data):
    """Yield (ext_name, section_idx, offset, kind) for every real, placed symbol.
    ext_name keeps the leading '_' (the filter); kind is 'func'|'data'."""
    secs = data.get("sections", [])
    seckind = {i: ("func" if s.get("contents") == "code" else "data")
               for i, s in enumerate(secs)}

    # exports (public)
    for i, s in enumerate(secs):
        for e in s.get("exports", []):
            yield e["name"], i, e["offset"], seckind.get(i, "data")

    # debug symbols: Func/Var (C) carry externalName; Label (asm) carry name.
    for s in data.get("debugSymbols", []):
        t = s.get("type")
        if t not in ("Func", "Var", "Label"):
            continue
        sec = s.get("section")
        off = s.get("address")
        if sec is None or off is None:
            continue                      # stack locals etc.
        name = s.get("externalName") or s.get("name")
        yield name, sec, off, seckind.get(sec, "data")


def collect_object_symbols(data):
    """Resolve one object's symbols to absolute addresses via self-anchoring.
    Returns (resolved list of (addr, name, kind), warnings)."""
    warnings = []
    # (section_idx) -> list of (ext_name, offset, kind)
    per_section = defaultdict(list)
    for ext_name, sec, off, kind in candidates_from_object(data):
        if not ext_name.startswith("_"):
            continue                      # drops LAB_/LP_GEN_/'P'
        per_section[sec].append((ext_name, off, kind))

    resolved = {}                         # addr -> (name, kind)
    for sec, syms in per_section.items():
        # anchor: base = addr_in_name - offset, must agree across the section
        bases = defaultdict(int)
        for ext_name, off, _ in syms:
            a = parse_addr(ext_name[1:])
            if a is not None:
                bases[a - off] += 1
        if not bases:
            miss = [ext_name for ext_name, _, _ in syms]
            warnings.append(f"section {sec}: no address-encoded anchor; "
                            f"skipped {len(miss)} symbol(s): {miss[:4]}")
            continue
        base = max(bases, key=bases.get)
        if len(bases) > 1:
            warnings.append(f"section {sec}: anchors disagree {dict(bases)}; "
                            f"using 0x{base:08x}")
        for ext_name, off, kind in syms:
            addr = base + off
            name = ext_name[1:]
            prev = resolved.get(addr)
            if prev and prev[0] != name:
                warnings.append(f"0x{addr:08x}: {prev[0]} vs {name} (kept first)")
                continue
            resolved[addr] = (name, kind)
    return resolved, warnings


def collect_sdk_defines():
    """SDK functions from the linker define template -> {addr: (name, 'func')}."""
    out = {}
    pat = re.compile(r"^define\s+_(\w+)\((8[cC][0-9A-Fa-f]{6})\)")
    for line in SUB_FILE.read_text().splitlines():
        m = pat.match(line.strip())
        if m:
            out[int(m.group(2), 16)] = (m.group(1), "func")
    return out


# ---------------------------------------------------------------------------
# Apply (live GhidraMCP plugin HTTP API)
# ---------------------------------------------------------------------------

def post(path, payload):
    req = urllib.request.Request(
        GHIDRA_URL + path,
        data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=30) as r:
        body = r.read().decode()
    try:
        return json.loads(body)
    except json.JSONDecodeError:
        return {"raw": body}


def is_ok(resp):
    if not isinstance(resp, dict):
        return True
    return resp.get("status") == "success" or resp.get("success") is True


def rename_function(addr, name):
    a = f"0x{addr:08x}"
    r = post("/rename_function_by_address",
             {"function_address": a, "new_name": name,
              "program": PROGRAM, "strict_mode": "off"})
    if is_ok(r):
        return "renamed"
    if "no function" in json.dumps(r).lower():
        post("/create_function", {"address": a, "program": PROGRAM})
        r = post("/rename_function_by_address",
                 {"function_address": a, "new_name": name,
                  "program": PROGRAM, "strict_mode": "off"})
        return "created" if is_ok(r) else f"error:{r}"
    return f"error:{r}"


def apply_labels(items):
    ok = err = 0
    CHUNK = 500
    for i in range(0, len(items), CHUNK):
        chunk = items[i:i + CHUNK]
        r = post("/batch_create_labels",
                 {"labels": [{"address": f"0x{a:08x}", "name": n}
                             for a, n in chunk],
                  "program": PROGRAM})
        r = r or {}
        ok += (r.get("labels_created", 0) or 0) + (r.get("labels_skipped", 0) or 0)
        err += (r.get("labels_failed", 0) or 0)
    return ok, err


# ---------------------------------------------------------------------------

def main():
    pairs = dump_inspect_json()

    funcs = {}                            # addr -> name
    data = {}                             # addr -> name
    all_warn = []
    for rel, jpath in pairs:
        d = json.loads(jpath.read_text())
        resolved, warnings = collect_object_symbols(d)
        for addr, (name, kind) in resolved.items():
            tgt = funcs if kind == "func" else data
            if addr in tgt and tgt[addr] != name:
                all_warn.append(f"{rel} 0x{addr:08x}: {tgt[addr]} vs {name}")
            tgt[addr] = name
        all_warn += [f"{rel}: {w}" for w in warnings]
    shutil.rmtree(DUMP_DIR, ignore_errors=True)

    # SDK defines are functions; they must not collide with data.
    for addr, (name, _) in collect_sdk_defines().items():
        funcs.setdefault(addr, name)

    # an address can't be both; functions win.
    for addr in list(data):
        if addr in funcs:
            del data[addr]

    print(f"[*] functions: {len(funcs)}   data labels: {len(data)}   "
          f"warnings: {len(all_warn)}")
    for w in all_warn[:40]:
        print("    !", w)

    if not APPLY:
        print("\n[dry run] re-run with --apply to write to Ghidra.")
        for addr in sorted(funcs)[:5]:
            print(f"    func 0x{addr:08x} {funcs[addr]}")
        for addr in sorted(data)[:5]:
            print(f"    data 0x{addr:08x} {data[addr]}")
        return

    print("[*] applying functions ...")
    stats = defaultdict(int)
    for n, addr in enumerate(sorted(funcs), 1):
        res = rename_function(addr, funcs[addr])
        stats[res.split(":")[0]] += 1
        if res.startswith("error"):
            print(f"    ! 0x{addr:08x} {funcs[addr]}: {res}")
        if n % 200 == 0:
            print(f"    ... {n}/{len(funcs)}")
    print(f"    functions: {dict(stats)}")

    print("[*] applying data labels ...")
    ok, err = apply_labels(sorted(data.items()))
    print(f"    data labels: ok={ok} err={err}")


if __name__ == "__main__":
    main()
