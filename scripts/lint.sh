#!/usr/bin/env bash
# Naming/header conventions + dead .IMPORT/.EXPORT check.
# See AGENTS.md "Naming Conventions" and scripts/check_naming.py's docstring.
#
# Every check runs even when an earlier one reports violations, and the exit
# status is decided at the end: a standing violation in one check must not
# silently skip the checks after it.
set -e

# The naming rules trace every symbol back to an address in the original binary,
# so these checks want the default (Japanese) build, not --lang en.
python3 build.py

set +e
fail=0

echo "=== check_naming.py ==="
python3 scripts/check_naming.py || fail=1

echo
echo "=== check_private_decls.py ==="
python3 scripts/check_private_decls.py || fail=1

echo
echo "=== clear_unused_imports.py --dry-run ==="
unused=$(python3 scripts/clear_unused_imports.py --dry-run src/asm/decompiled/*.src)
echo "$unused"
if echo "$unused" | grep -q "^Would remove [1-9]"; then
  echo "FAIL: unused .IMPORT/.EXPORT directives found -- run scripts/clear_unused_imports.py to fix"
  fail=1
fi

# Informational: never blocks, but a non-zero exit here means the script itself
# broke rather than that it found mismatches.
echo
echo "=== check_test_naming.py (informational) ==="
python3 scripts/check_test_naming.py || echo "NOTE: check_test_naming.py exited non-zero"

echo
if [ $fail -ne 0 ]; then
  echo "FAIL: one or more checks reported violations"
else
  echo "All blocking checks passed"
fi
exit $fail
