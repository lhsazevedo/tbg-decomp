# Rev A

What changed in `1ST_READ.BIN` between the original Japan release (the one this
project decompiles) and Rev A.

| | IP.BIN | `1ST_READ.BIN` |
|---|---|---|
| Original | T35402M V1.003, 1999-11-24 | 970,016 bytes |
| Rev A | T35402M V2.000, 2000-01-06 | 970,400 bytes (+0x180) |

The "Tokyo Bus Guide - Bijin Bus Guide Tenjou Disc" is not a build of the game:
it is a separate Sofdec movie player (T35404M "BUSNAVI", 2000-11-22, `BUS_*.SFD`)
and shares no game objects.

## Method

```sh
gdworkbench track extract "<Track 3>.bin" -o revA
dcmatch build-db build/output_matching/src --sdk "TBG matching" -o tbg.json
dcmatch merge ../dcmatch/db.json tbg.json -o all.json
dcmatch scan revA/root/1ST_READ.BIN --db all.json -o revA.json
```

Unplaced objects were then diffed with `sh-elf-objdump`, resolving literal-pool
pointers to our symbols through each section's placement delta.

dcmatch verifies code only. An object's data sections are located through its
relocations, not hashed, so a "placed" C/D section can still differ (see
course_menu below).

## Summary

- SDK libraries are identical (5,054 objects placed in the original, 5,045 in
  Rev A; the 9 missing are ours).
- 59 of 69 game objects with code are identical modulo relocations.
- Code changed in 10 units: `0129cc_game`, `013ae8_route`, `014934_unused_load`,
  `018644_file_select`, `01b19c_system_menu`, `01fa78_hud`, `024280_bus_input`,
  `027958_bus_draw`, `02b464_grading`, `02c884_stop`.

## Code changes

| Function | Rev A |
|---|---|
| `RouteSpawnLoadTask`, `UnusedLoadSpawnTask` | Also zero `var_viewParams_8c18ad28` and `var_routeModelIndexes_8c18adb0` |
| `SystemMenuWriteToVmu` | Run report / unlock update only when `var_gameMode == 0` |
| `drawMirror` | Extra branch for `var_timeOfDay == 2`: `njCnkSetSimpleLightIntensity` / `njCnkSetSimpleLightColor` before drawing |
| `HudSpawnTask` | Also initializes `var_hudMark_8c2264a8+0x8` |
| `BusInputUpdate` | +0x1c bytes; drops a `njMultiMatrix` call, reads `var_cameraHeightDelta_8c227de8` |
| `gradeLaneUse` | Substantially reworked (+0x40 bytes, different `var_busState` fields and thresholds) |
| `GameInit` | No longer touches `var_8c226434` / `var_8c226438` |
| `BusDrawPlaceEntity`, `StopUpdateArrival`, `gradeIntersection`, `enterDriveEnd`, `segmentReloadTask` | Smaller logic changes, not yet characterized |

`GameStartSelectedCourse` and `StopSetup` differ only in register allocation and
scheduling as far as the diff shows; not verified line by line.

## Data changes

`016d2c_course_menu` code is unchanged, but its C section gains two trial-version
lines after 「さぁ、コースを選んでくれ」:

- 「この体験版では３つのコースの中の一部分を<E>走ることができるぞ」
- 「ここで運転のこつをつかんだら<E>ぜひ製品版に挑戦してみてくれ」

Other changed data: `0129cc_game` C (+3 bytes), `01b19c_system_menu` C (+1),
an extra 0x24 bytes of D between `016d2c_course_menu` and `0193c8_vm_select`, and
same-size D content changes in `01fa78_hud`, `024280_bus_input` and
`02b464_grading`. BSS sizes are unchanged.
