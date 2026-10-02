# Validating delinker TU boundaries

The delinker recovers translation-unit boundaries from a linked image using
literal-pool references, relative branches and function alignment. Those are the
only signals in the *instructions*, and they can only ever find a boundary that
looks plausible -- they cannot tell a real TU boundary from an arbitrary aligned
function start, so the split is a hypothesis, not a fact.

Two kinds of evidence survive linking and can test that hypothesis after the
fact: section layout, and cross-unit reference structure.

## What the layout can testify to

`build/lnk_matching_template.sub` ends with `start IP(8C008000),DSGLH(8C010000)`
-- every other section is given no start, so every section is laid out by
concatenating each input object's contribution **in link order**, which is the
`SRCS` order, which is address order. Two consequences:

- **One object contributes one contiguous run per section.** So a unit's
  exclusively-referenced symbols *bracket* its ownership: everything between its
  lowest and highest exclusive symbol belongs to the same object.
- **Data order must follow code order.** If unit A's code precedes unit B's,
  A's C/D/B contributions must precede B's.

The authoritative view is the section table in `build/output_matching/tbg.map`,
which lists each section's contributions per object in link order. Read that
rather than inferring -- a stale map will happily describe a layout that no
longer exists.

**An ordering test can only disprove a split, never a merge.** If two adjacent
units were really one TU, their combined data is still correctly ordered. Run
across the project, section C and D come back perfectly ordered for all 48 units
that have data -- which establishes that no boundary is grossly *misassigned*,
and says nothing about whether adjacent units should be joined.

## The three tests that do find false splits

1. **No private state.** A unit with zero exclusively-referenced bss, zero
   exclusive const/data, and all of its exports consumed by a single other unit
   is not behaving like a translation unit -- it is behaving like the private
   half of one.
2. **Code contiguity.** A TU occupies one contiguous address range, so a suspect
   can only be merged with a unit that is its *immediate neighbour*. This is what
   eliminates most of the false positives from test 1: a small TU whose only
   caller happens to sit far away is perfectly normal.
3. **Pair-private data.** A variable referenced by exactly the two candidate
   units and nobody else would become a file-static on merge. Two genuinely
   separate TUs have no reason to share a variable no third unit can see.

## Call encoding

SHC calls a same-file function with BSR whenever it is in range (+-4KB), and
uses a pool load + JSR only when it is out of range or in another file. So a
JSR (or tail-call JMP) to a callee within BSR range proves the two were in
different TUs. Taking a function's address (tasks, callbacks) is always an
absolute pool entry and proves nothing.

Checked by `tbg-experiments/tu-calls/tu_calls.py`. Found this way: objects split
at `0289ac` (`pedestrianTask` JSRs to the crossing helpers at `0289xx` but BSRs
to `advancePedPathPos_8c0289ac`), and `016108_resgrp_free` separate from
`01614c_replay_menu`.

## Results

Suspects surviving all three tests, strongest first:

| candidate | fns | pair-private bss |
|---|---|---|
| `02d968_stop_spawn` + `02d19c_passenger` | 1 | 7 -- the six board/exit spots + `passengerActed` |
| `02b2f0_drive_msg` + `02b464_grading` | 2 | 1 -- `var_driveMsgQueue_8c228564` |
| `022bdc_bus` + `023310_bus_init` | 1 | 1 -- `var_busDoorLastFrame_8c227db4` |
| `025b98_traffic_drive` + `026710_traffic` | 2 | 0 |
| `012324_peripheral_support` + `012504_input` (merged as `012324_input`) | 1 | 0 |

Eliminated by contiguity, i.e. small TUs whose sole consumer is far away:
`02412c_bus_line` (2 from `022bdc_bus`), `023310_bus_init` (31 from
`012f44_game`).

`04ce10_line_nodes` flags on test 1 but is a genuine **data-only object**: no P
section at all, which is also why `013ae8_route` uses data that links
nowhere near `013ae8`'s own.

`0129cc_pause` + `012f44_game` (merged as `0129cc_game`) were found by layout
instead: pause had no data object of its own, while its private D
(`init_pauseDimQuad`) and bss (`pauseSettle`..`onRetire`) sat in the middle
of game's D and B runs. Pause links first, so as separate TUs all of its data
would precede game's.

`016bf4_demo_input` + `01614c_debug_menu` (merged as `01614c_replay_menu`) were
found by pairing: demo_input's one function is the playback half of replay_menu's
`ReplayMenuDemoRecordTask`, game spawns both, and replay_menu's reset zeroes
`demoPrevOn`, which only playback reads. Its sole consumer being game is what
test 2 had flagged. Layout can't separate them: split, demo_input would own
`demoCursor`/`demoPrevOn` at the same addresses.

`020528_drive_cue_init` + `020214_drive_cue_task` (merged as `020214_drive_cue`)
are a task and its sole installer: init spawns the task and seeds its state.
The pusher sits right after the task and neither has data, so layout can't
separate them.

`0222dc_fadecmd` was re-split at `02239c`: its first function went to
`021b9c_tile_draw`, the rest to `022464_render`. `TileDrawSpawnTask_8c0222dc` is
the only pusher of tile_draw's task and the only writer of its light vars. SHC
calls same-file functions with BSR when in range, yet the task reaches
`RenderQueueDraw`/`RenderQueueDraw2` via JSR from ~540 bytes away, so the queue
functions were not in its file; they only touch the draw-command queues in
fade's B run.

`014f54_text` was split at `015034` into `014f54_sprite` and `015034_text`.
`TxtDrawTextBox_8c0155e0` BSRs to `unpackGlyph_8c015110` but reaches
`SpriteDraw_8c014f54`, 2464 bytes away and well within BSR range, via JSR, so
the sprite functions were in another file. `015034` is the only boundary that
fits: 4-aligned, and no PC-relative reference crosses it. The sprite half has
no data of its own.

`028258_objects` was split at `0289ac` into `028258_signal` and
`0289ac_objects`. `pedestrianTask_8c028e00` BSRs to `advancePedPathPos_8c0289ac`
but reaches `SignalGetFrame_8c028900`, `SignalMarkPedCrossing_8c02897a` and
`SignalIsCrossingOccupied_8c02898e`, 1.3-1.7 KB away, via JSR, and
`pedestriansTask_8c0293f6` JSRs to `SignalClearPedCrossingFlags_8c02890c`, so
the signal functions were in another file. `0289ac` is the only boundary that
fits. The signal half has no C/D data, and its B vars all sit before the
objects half's.

`0289ac_objects` was split again at `02a9fc` into `0289ac_objects` and
`02a9fc_message_box`. No call or data reference crosses `02a9fc` in either
direction, and each half's C, D and B data are contiguous runs, the message
box's after objects' (C from `8c03a17c`, D from `8c046758`, the last six B
vars from `8c228478`). `02a9fc` is 4-aligned and no PC-relative reference
crosses it.

## `02d968_stop_spawn`: the case in detail

- One function, 253 lines, **zero** private bss, one D blob.
- Calls all five of `02d19c`'s public functions -- and is the only file that does.
- `02d19c` calls back into it; the sole outside entry is one call from `0129cc_game`.
- Initialises `02d19c`'s spot vars with literals, then transforms them by the bus matrix.
- Their D blobs are exactly adjacent: `init_seatPositions` is 248 bytes and the
  gap to `init_passengerVoiceVariant` is 248.
- Address order is what the merged source would produce: static task callbacks
  first (`0x02d19c`-`0x02d8f0`), the public init last at `0x02d968` -- the order
  required if the callbacks are file-static and spawned by address.

Under a merge everything resolves at once: the five exports become statics, the
nine bss vars at `0x228910`-`0x228974` become file-statics, and the
"TU with no data of its own" anomaly disappears.

Not proven. What was checked and did *not* decide it: section D is consistently
ordered (a merge would be too), the boundary is a clean aligned function start
(true of both), and no literal pool straddles it. The `.IMPORT`s cannot help at
all -- the delinker synthesises them from the boundary it chose, so they are a
consequence of the hypothesis, not evidence for it. Compare the section B labels
in `lessons_learned.md`: the same class of artifact.
