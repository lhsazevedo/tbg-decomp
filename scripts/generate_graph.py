#!/usr/bin/env python3
"""Build the cross-unit reference graph from the compiled objects.

Runs `sh4objtest inspect --format=json` on every built object, then resolves each
object's external relocations to the unit that defines the target symbol. Every edge
is classified by the *target* symbol's kind -- "code" (a function, defined in a code
section) or "data" (a global, defined in a data section) -- which is the distinction
the aggregate `unit -> {unit: count}` graph used to throw away.

Runs inside the Docker image (sh4objtest lives there); see scripts/generate-graph.sh.

Outputs:
  build/inspect_graph/graph.json  -- nodes + classified edges + SDK imports
  build/inspect_graph/graph.html  -- self-contained viewer (graph.json inlined)

Node ids mirror the object layout:
  build/output/src/<stem>.obj      (C-decompiled units) -> c_<stem>
  build/output/src/asm/<stem>.obj  (still-asm units)    -> asm_<stem>
"""

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
C_OBJ_DIR = REPO_ROOT / "build" / "output" / "src"
ASM_OBJ_DIR = C_OBJ_DIR / "asm"
OUT_DIR = REPO_ROOT / "build" / "inspect_graph"
TEMPLATE = REPO_ROOT / "scripts" / "graph-template.html"


def node_id(obj_type, stem):
    return f"{obj_type}_{stem}"


def discover_objects():
    """Yield (node_id, type, Path) for every built object."""
    for obj in sorted(C_OBJ_DIR.glob("*.obj")):
        yield node_id("c", obj.stem), "c", obj
    for obj in sorted(ASM_OBJ_DIR.glob("*.obj")):
        yield node_id("asm", obj.stem), "asm", obj


def inspect(obj):
    out = subprocess.run(
        ["sh4objtest", "inspect", str(obj), "--format=json"],
        capture_output=True, text=True, check=True,
    ).stdout
    return json.loads(out)


def build_graph(dump_dir=None):
    dumps = {}          # node_id -> parsed inspect json
    node_type = {}      # node_id -> "c" | "asm"
    for nid, ntype, obj in discover_objects():
        data = inspect(obj)
        dumps[nid] = data
        node_type[nid] = ntype
        if dump_dir:
            (dump_dir / f"{nid}.json").write_text(json.dumps(data, indent=1))

    # symbol -> (owner_node, kind); kind from the defining section's contents.
    owner = {}
    for nid, data in dumps.items():
        for sec in data.get("sections", []):
            kind = "code" if sec.get("contents") == "code" else "data"
            for exp in sec.get("exports", []):
                owner[exp["name"]] = (nid, kind)

    nodes = []
    for nid, data in dumps.items():
        secs = data.get("sections", [])
        n_exports = sum(len(s.get("exports", [])) for s in secs)
        nodes.append({
            "id": nid,
            "type": node_type[nid],
            "hasCode": any(s.get("contents") == "code" for s in secs),
            "hasData": any(s.get("contents") != "code" for s in secs),
            "nExports": n_exports,
        })

    # (source, target, kind) -> {count, symbols}
    edges = {}
    sdk_imports = {}
    for nid, data in dumps.items():
        for sec in data.get("sections", []):
            for rel in sec.get("externalRelocations", []):
                name = rel["name"]
                resolved = owner.get(name)
                if resolved is None:
                    sdk_imports[name] = sdk_imports.get(name, 0) + 1
                    continue
                target, kind = resolved
                if target == nid:
                    continue  # self-reference across sections; not a cross-unit edge
                key = (nid, target, kind)
                slot = edges.setdefault(key, {"count": 0, "symbols": set()})
                slot["count"] += 1
                slot["symbols"].add(name)

    edge_list = [
        {"source": s, "target": t, "kind": k,
         "count": v["count"], "symbols": sorted(v["symbols"])}
        for (s, t, k), v in sorted(edges.items())
    ]

    return {
        "nodes": sorted(nodes, key=lambda n: n["id"]),
        "edges": edge_list,
        "sdkImports": dict(sorted(sdk_imports.items())),
    }


def render_html(graph):
    template = TEMPLATE.read_text()
    injected = "<script>const GRAPH = " + json.dumps(graph) + ";</script>"
    marker = "<!-- GRAPH_DATA -->"
    if marker not in template:
        sys.exit(f"template {TEMPLATE} is missing the {marker!r} placeholder")
    return template.replace(marker, injected)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dump-dir", nargs="?", const=str(REPO_ROOT / "build" / "inspect"),
                    help="also write per-object inspect JSON here (default build/inspect)")
    args = ap.parse_args()

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    dump_dir = None
    if args.dump_dir:
        dump_dir = Path(args.dump_dir)
        dump_dir.mkdir(parents=True, exist_ok=True)

    graph = build_graph(dump_dir)

    (OUT_DIR / "graph.json").write_text(json.dumps(graph, indent=1))
    (OUT_DIR / "graph.html").write_text(render_html(graph))

    n_code = sum(1 for e in graph["edges"] if e["kind"] == "code")
    n_data = sum(1 for e in graph["edges"] if e["kind"] == "data")
    print(f"Wrote {OUT_DIR}/graph.json and graph.html")
    print(f"  {len(graph['nodes'])} nodes, {len(graph['edges'])} edges "
          f"({n_code} code, {n_data} data), {len(graph['sdkImports'])} SDK imports")


if __name__ == "__main__":
    main()
