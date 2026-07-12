#!/bin/bash
# Regenerate progress.png / progress.svg from scripts/generate-chart.py.
# Builds a small python+matplotlib image (cached after first run) and renders
# the chart headlessly into the repo root.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

docker build -t tbg-chart -f "$REPO_ROOT/scripts/chart.Dockerfile" "$REPO_ROOT"

docker run --rm \
    -v "$REPO_ROOT:/app" \
    -w /app \
    tbg-chart \
    python scripts/generate-chart.py
