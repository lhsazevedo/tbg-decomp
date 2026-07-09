#!/usr/bin/env bash
# Naming/header conventions + dead .IMPORT/.EXPORT check.
# See AGENTS.md "Naming Conventions" and scripts/check_naming.py's docstring.
set -e

make clean all

echo "=== check_naming.py ==="
python3 scripts/check_naming.py

echo
echo "=== clear_unused_imports.py --dry-run ==="
python3 scripts/clear_unused_imports.py --dry-run src/asm/*.src src/asm/decompiled/*.src
if python3 scripts/clear_unused_imports.py --dry-run src/asm/*.src src/asm/decompiled/*.src | grep -q "^Would remove [1-9]"; then
  echo "FAIL: unused .IMPORT/.EXPORT directives found -- run scripts/clear_unused_imports.py to fix"
  exit 1
fi
