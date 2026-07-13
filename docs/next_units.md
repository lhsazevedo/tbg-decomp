# Next Decompilation Targets

Snapshot from a relocation-graph analysis (2026-07-12, right after finishing
`013ae8_route_load.c`). Method: `sh4objtest inspect --format=json` on every
built object (`build/output/src/*.obj` + `build/output/src/asm/*.obj`), map
exports to owning units, then external relocations give unit-to-unit edges.
Re-run the analysis when this list goes stale; it only takes a few minutes.

Ranking criteria used: continuation of the route-load pipeline, plus fan-in
from already-decompiled units.

## 1. `02171c` -- tile/segment streaming engine (1.1 KB, 8 functions)

Most direct continuation of route_load. `FUN_8c021810`/`FUN_8c02190a` are the
per-frame callbacks route_load hands to `AsqProcessQueues_8c011fe0`;
`FUN_8c02175a`/`FUN_8c021a24` run at post-load/teardown. Imports are exactly
the streaming surface: `var_currentTileRegionList_8c226534`,
`var_datFiles_8c18adb4`, `var_pvrDir_8c18ad4c`, `AsqRequestTexlist_8c01181c`,
`njReadBinary`/`njReleaseTexture`.

**Gain:** confirms the `tileRegionList_0x0c` element layout (marked
"unconfirmed" in `CourseSegment`), likely resolves several `ukn_` fields,
completes the load pipeline end-to-end.

## 2. `02af78` -- story event selection (888 B, 8 functions) -- DONE

`pickSegmentEvent_8c02b170` is called in every route_load post-load block.
Resolved: this is the story-cutscene-event picker. Each route has an
`EventEntry` table (`init_8c04b1f0`/`init_8c04abb0`/`init_8c04b920` for
Shinjuku/Wangan/Ome), entries carry a time-of-day field plus a packed
day-of-week mask and prerequisite/action condition codes;
`scanEventCandidates_8c02b03c` filters by tod+day+progress each course load,
`pickSegmentEvent_8c02b170` narrows to the current segment and randomly
selects one, `applyEventFlags_8c02b292` applies its post-conditions. See
`docs/gameplay.md` for the resolved open question this answers.

**Gain:** confirmed how cutscenes/events get armed per course/segment
(previously an open question left by route_load).

## 3. `026710` -- traffic vehicle spawner (4.4 KB, 13 functions)

Consumes `slots_0x04[8]` = `*_mac_cpu1.dat` (CPU-vehicle placement). Uses
trig (`njSin`/`njCos`/`acosf`), the Asq RNG, pushes tasks. Calls into
`02c884` (`FUN_8c02cd6a`); route_load calls `02c884`'s `FUN_8c02caba` in the
same post-load block, so **`02c884` (2 KB, 9 functions) is a natural
follow-up pairing**.

**Gain:** names the `_cpu` asset category consumers end-to-end; connects to
the vehicle model code tables (`3s_`/`3t_`, sed/tax/tor...) in route_load's
data.

## 4. `022464` -- screen fade tasks (1.9 KB, 8 functions, half-named already)

Not load-related but best leverage-per-byte: 6 decompiled units
(`016d2c_course_menu`, `019e98_main_menu`, `0193c8_vm_menu`, `015ab8_title`,
`01d290_album`, `012f44`) extern `push_fadein_8c022a9c` /
`push_fadeout_8c022b60`; 16 units reference it overall. Exports already
meaningfully named (`task_fadein`, `draw`, ...).

**Gain:** mostly mechanical quick win; closes a hole every menu depends on.

## 5. `028258` -- pedestrians + scene objects + message box (11.5 KB, 41 functions)

Highest-fan-in code unit remaining (19 referencing units, 6 decompiled). Owns
route_load's remaining mysteries: the pedestrian chain (slots 11/12 =
`*_mac_hum_g0/m0.dat`, `task_pedgroup_8c029078`, `task_pedestrians_8c0293f6`),
the typed scene-object streamer `FUN_8c029ad4` (type 6 = O_FUMI railroad
crossing), and the menu message-box API (`swapMessageBoxFor_8c02aefc`,
`menuTextboxText_8c02af1c`) used by course_menu.

**Gain:** biggest payoff by far, but 10x the size of the others -- better
after one or two small ones, or carved into the pedestrian half first.

## Honorable mention

- `0222dc` (392 B, 4 functions): referenced by 12 asm units, trivial to knock
  out, but only `012f44` among decompiled units uses it.
- `01614c` (3.6 KB): VMU save / demo-recording related (`BupGetInfo/Mount/
  Unmount`, demo buffers); 4 decompiled callers.
- `01bb48` (3.6 KB): VMU LCD (`vmsLcd_8c01c8fc/8c01c910`); 5 decompiled
  callers.

## Suggested order

`02171c` -> `022464` (palate cleanser) -> `026710`+`02c884` -> `028258`.
