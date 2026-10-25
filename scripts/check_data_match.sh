#!/usr/bin/env bash
# Builds the reference (matching) objects and the plain-build objects (serial
# logging off, as its debug-name string tables would cause false-positive
# diffs), then diffs each decompiled unit's data sections
# (C/D/B; P is code and expected to differ) via dcdiff.py.
#
# The set of checked units is derived: every unit built from C
# (build/output/src/*.obj) that also has an archived decompiled-asm reference
# (build/output_matching/src/asm/decompiled/<unit>.obj). Such a unit is expected
# to data-match, and a diff FAILS the build.
#
# NOT_MATCHING is the allowlist of exceptions -- units not yet data-matching,
# where a diff is only logged. Drop a unit from this list once its data matches.
set -e

NOT_MATCHING="
"

python3 build.py matching
python3 build.py

fail=0

is_allowlisted() {
  for u in $NOT_MATCHING; do [ "$u" = "$1" ] && return 0; done
  return 1
}

for c_obj in build/output/src/*.obj; do
  unit=$(basename "$c_obj" .obj)
  ref_obj="build/output_matching/src/asm/decompiled/${unit}.obj"
  [ -f "$ref_obj" ] || continue   # no decompiled-asm reference to match against

  echo "=== $unit ==="
  if python3 scripts/dcdiff.py "$ref_obj" "$c_obj"; then
    echo
    continue
  fi

  if is_allowlisted "$unit"; then
    echo "NOTE: $unit not data-matching yet (allowlisted), ignoring diff"
  else
    echo "FAIL: $unit is expected to data-match but C/D/B differ"
    fail=1
  fi
  echo
done

echo "=== Imported symbols ==="
python3 scripts/check_imports.py || fail=1
echo

echo "=== Summary ==="
if [ $fail -eq 0 ]; then
  echo "All checked units data-match and import what the archive imports"
else
  echo "Some checked units do not data-match, or diverge on imports"
fi

exit $fail
