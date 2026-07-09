#!/usr/bin/env bash
# Compiles each decompiled unit's reference asm (matching flags) and its C source
# (production flags, minus SERIAL_DEBUG so debug-only data doesn't cause false
# positives) independently of any other build, then diffs their data sections
# (C/D/B; P is code and expected to differ) via dcdiff.py.
#
# CONFIRMED units are known to match and fail the build if they regress.
# Everything else is checked too, but only logged -- those units aren't
# confirmed data-matching yet, so a diff there is not (yet) a build error.
set -e

ASMSH_FLAGS="-define=MATCHING=1 -debug=d -cpu=sh4 -endian=little -sjis"
OUT=build/tmp/datacheck
CONFIRMED="013ae8_route_load 016d2c_course_menu"
ALL="012324_peripheral_support 014f54_text 0100bc_sound 010fe8_heap 015ab8_title 0193c8_vm_menu 0207d4 016c58_prompt 012f44 011120_asset_queues 019e98_main_menu 016d2c_course_menu 012504_input 016bf4_demo_input 01d290_album 013ae8_route_load"

rm -rf "$OUT"
mkdir -p "$OUT"

fail=0

is_confirmed() {
  for u in $CONFIRMED; do [ "$u" = "$1" ] && return 0; done
  return 1
}

for unit in $ALL; do
  ref_obj="$OUT/${unit}_ref.obj"
  c_obj="$OUT/${unit}_c.obj"

  wibo "$SHC_BIN/asmsh.exe" $(echo "src/asm/decompiled/${unit}.src" | tr / '\\') -object="$(echo "$ref_obj" | tr / '\\')" $ASMSH_FLAGS > /dev/null
  wibo "$SHC_BIN/shc.exe" $(echo "src/${unit}.c" | tr / '\\') -object="$(echo "$c_obj" | tr / '\\')" -sub=build/shc_datacheck.sub > /dev/null

  echo "=== $unit ==="
  if python3 scripts/dcdiff.py "$ref_obj" "$c_obj"; then
    status=ok
  else
    status=diff
  fi

  if [ "$status" = diff ]; then
    if is_confirmed "$unit"; then
      echo "FAIL: $unit is a confirmed data-matching unit but C/D/B differ"
      fail=1
    else
      echo "NOTE: $unit not confirmed data-matching yet, ignoring diff"
    fi
  fi
  echo
done

exit $fail
