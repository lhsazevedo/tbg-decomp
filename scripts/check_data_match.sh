#!/usr/bin/env bash
# Builds the reference (matching) objects and the plain-build objects with
# serial logging disabled (its debug-name string tables would otherwise cause
# false-positive diffs), then diffs each decompiled unit's data sections
# (C/D/B; P is code and expected to differ) via dcdiff.py.
#
# CONFIRMED units are known to match and fail the build if they regress.
# Everything else is checked too, but only logged -- those units aren't
# confirmed data-matching yet, so a diff there is not (yet) a build error.
set -e

CONFIRMED="013ae8_route_load 016d2c_course_menu 01d290_album"
ALL="012324_peripheral_support 014f54_text 0100bc_sound 010fe8_heap 015ab8_title 0193c8_vm_menu 0207d4 016c58_prompt 012f44_game 011120_asset_queues 019e98_main_menu 016d2c_course_menu 012504_input 016bf4_demo_input 013ae8_route_load"

make -f Makefile.matching clean all
make clean
make SERIAL_DEBUG=0 all

fail=0

is_confirmed() {
  for u in $CONFIRMED; do [ "$u" = "$1" ] && return 0; done
  return 1
}

for unit in $ALL; do
  ref_obj="build/output_matching/src/asm/decompiled/${unit}.obj"
  c_obj="build/output/src/${unit}.obj"

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
