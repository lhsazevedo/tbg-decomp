#!/bin/bash
# Regenerate build/inspect_graph/graph.json + graph.html from the built objects.
# Runs the generator inside the Docker image (sh4objtest lives there); builds first
# so the objects exist. Open the resulting graph.html directly in a browser.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

./docker-run.sh make graph
