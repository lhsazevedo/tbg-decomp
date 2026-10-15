# Gameplay Reference

How the game actually plays, from someone who has played it. Useful context
when naming functions/data: the code was written to implement *this*.

Tokyo Bus Guide (1999, Dreamcast): a Japanese bus simulator. You drive a city
bus along a predefined route with predefined passenger stop requests.

## Modes

- **Main modes** (3 routes x 3 times of day = 9 courses; see
  `reference_route_assets` / `013ae8_route.c` for the courseId scheme):
  - **Story mode**: at some stops, after the interior view, a cutscene event
    may trigger showing passenger dialog: pre-rendered slideshows with audio
    and textboxes. Seen events unlock passenger profiles in the event menu
    ("PROFILE FILE" notebook UI -- grid of portraits, per-character bio pages
    with a checkbox grid of seen events). This is NOT `01d290_album`: the
    album is a separate screen holding six passenger letters in a 3x2 grid
    (`letters_0x2c`), one handed out at random every seventh day by
    `016d2c_course_menu`. The event menu is `01c980_profile_file`: 55 slots
    (5 full rows of 10 plus a 5-wide row 5), each unlocked by any one of a
    list of `02af78_event` progress flags, each row's bio art in its own
    `prof0N.pvm`.
  - **Free run**: same courses, no cutscenes/events.
- **Practice**: 11 runs teaching individual mechanics, each preceded by a
  slideshow guide. Runs take place on sections (segments) of the main route
  maps. Lesson N loads courseId 27 + N (`lessonDescriptionTask_8c01e27c` hands
  `var_practiceLesson_8c22640c + 0x1b` to `GameStartCourse_8c013310`), so
  they are `init_courseTable_8c043ca4` entries 27-37.

## Driving loop

- Follow driving rules: speed limit, turn signals, traffic lights, smooth
  acceleration/braking, and the time schedule.
- Committing a penalty displays a text message on screen.
- A single **score bar** drains with penalties; gameplay ends if it empties.
  Passing bus stops (both boarding stops and drive-bys) replenishes it a bit.
- Reaching the final stop ends the run: score screen, then back to course
  select.
- The VMU LCD shows art during gameplay (see `01bb48` `vmsLcd_*`).
- **Route arrows** (yaji.njd): three green chevrons block off-course roads
  and point back to the route, lit one after another
  (`ObjectsInstallRouteArrows_8c029920`).

## Results bonuses

The BONUS screen (`ResultsShowPassedRun_8c01e0b4`, `01d7fc_results.c`) has six
rows; their sum is added to `exp_0x90`.

| Row | Variable | Value |
|---|---|---|
| 完走 (finish) | `var_scoreFinishBonus_8c2263ec` | `init_finishBonusTable_8c0451a0[route*3+tod]` on a passed run, else 0 |
| 初走 (first run) | `var_scoreFirstRunBonus_8c2263f0` | 100 if the course was never played before, pass or fail |
| driver's points x10 | `var_scoreDriverPointsBonus_8c2263f4` | driver points x 10 |
| バッジ獲得 (badge earned) | `var_scoreBadgeBonus_8c2263f8` | 50/100/200 for a new bronze/silver/gold badge |
| 乗客 (passengers) | `var_scorePassengerBonus_8c2263fc` | passengers x 5 |
| EVENT | `var_scoreEventBonus_8c226400` | events x 50 |

## Camera

- Cycled in-drive with the **Y button** (`BusCameraUpdate_8c025078`,
  `024b4c_bus_camera.c`). The active mode lives in `var_cameraMode_8c227d9c`
  (`BUS_CAMERA_*` in `024b4c_bus_camera.h`).
- The Y button only ever cycles `BUS_CAMERA_COCKPIT` (0) through
  `BUS_CAMERA_THIRD_PERSON_FAR` (3) -- confirmed player-facing:
  - **`BUS_CAMERA_COCKPIT` (0)**: camera sits exactly at the bus's own
    position; draws the front dashboard model (`drawFrontBusModel_8c024cc8`);
    yaws with an eased copy of the steering angle
    (`busState.cameraYawEase_0x3c8`) -- the sway felt when turning.
  - **`BUS_CAMERA_FIRST_PERSON` (1)**: camera offset a small fixed distance
    ahead of the bus along its heading; no bus model drawn at all (no
    dashboard); a gentler pitch-based bob than mode 0.
  - **`BUS_CAMERA_THIRD_PERSON_NEAR` (2)** / **`BUS_CAMERA_THIRD_PERSON_FAR`
    (3)**: chase camera via `positionCamera_8c024d6c`, follow distance 18 and
    30 respectively; draws the third-person bus model (`BusCameraDrawBusModel_8c024bb8`).
  - **`BUS_CAMERA_FIXED_TARGET` (4)**: fixed on `var_fixedCameraTarget_8c227d90`, aimed via
    `njPointCameraInterest`. Traced as unreachable: the Y-button cycle wraps
    at 3 back to 0 and no other decompiled code path ever assigns 4 to
    `var_cameraMode_8c227d9c` -- dead in the shipped game as far as traced,
    not merely hard to reach.
  - Modes 5-7 (`BUS_CAMERA_DEMO_*`) are attract-mode only
    (`CinematicUpdateCamera_8c025906`, `025870_cinematic.c`): the tour cuts between
    scripted shots as the bus passes stop markers, captioning each with its
    place name. The player never sees them.
- This confirms the four player-facing views the mode reads as, in cycle
  order: cockpit (bobs on the wheel with steering), first-person (no
  dashboard, gentle bob), third-person near, third-person far.
- Modes 0 and 1 additionally roll the camera by the road's pitch, taken from
  recent Y waypoint history -- this is the extra bob felt when turning on
  top of mode 0's yaw sway.
- The mirrors run a separate camera (`BusCameraUpdateMirror_8c025604`,
  `var_mirrorCamera_8c1bb944`), selected by `busState.mirror_0x268`.

## Turn signals

- Signalling is manual and player-driven; the game never signals for you.
  The state (`BusState.signalSide_0x25c`, formerly documented here as a
  "mirror button" -- see the Penalties section's naming note) is 0 = off,
  1 = left, 2 = right, set by the same two buttons that also switch the
  mirror view (`mirror_0x268`, `024280_bus_input.c:441-511`); it clears itself once a
  lane change completes (`02b464_grading.c`) or when auto drive mode
  finishes a lane change (`023938_bus_drive.c:213-219`).
- One control genuinely does three jobs: it is the driver's turn signal, the
  mirror-view selector, and the input the lane-change penalty check reads
  (`02b464_grading.c:577`). Naming it after any single one of those
  (e.g. a "mirror button" or a "turn signal") would misdescribe the other
  two, so the field is named for the shared thing it actually holds -- the
  driver's latched left/right side intent.
- At a stop the blinker seen while the doors are open is the player's own
  signal for pulling back into traffic -- there is no door-driven blinker.
  Failing to signal there is the `NO_SIGNAL` penalty.
- Signalling the wrong way is also penalised: the lane-change check compares
  the signal against the direction actually taken.
- The lamps are five models on the bus body, shown and hidden from the low
  five bits of `blinker_0x080` (`blinkerLights_0x02c`, `027958_bus_draw.c`).
  Not all five are blinkers: bit 0 is the brake lamp and bits 3/4 the night
  running lights, both forced on elsewhere. The tick SFX plays on the first
  frame of each signal.
- `HudState.driveMarkLatched_0x10` (`01fa78_hud.c`) is unrelated to
  the driver's signalling: it is a one-shot latch over
  `markDriveFlags_0x3b0`'s low 3 bits, which are not lamp bits at all.
  `BusTask_8c022bdc` fills `markDriveFlags_0x3b0` every frame from the
  attribute-grid record under the bus, so it carries the *map's* authored
  drive instructions, not the player's `signalSide_0x25c` or
  `blinker_0x080`.

## Reverse

- Engage by stopping and shifting into reverse; `busState.gear_0x2f4` holds 5
  for reverse (`023938_bus_drive.c`).
- Both blinkers flash together while in reverse -- a hazard-style double
  blinker toggled every 16 frames from bit 4 of
  `busState.hazardBlinkCounter_0x2f8` (`022bdc_bus.c:318`).

## Penalties

Penalties drain the score bar and show an instructor message. Not all are
traffic offenses -- roughly half are service errors, things a bus driver is
marked down for that have nothing to do with the road.

The messages are rows of `init_instructorDialogs_8c044c08`
(`016d2c_course_menu.c`), indices 32-63 of the `INSTR_*` enum in
`016d2c_course_menu.h`; the string ids are the `MSG_SEQ_*` block in
`strings_en_us.h`. Several have graded severities and multiple message
variants.

**Where penalties actually fire, and where they're shown.** In-drive,
`02b464_grading.c` grades every penalty through
`adjustPoints_8c02b464(msgSet, delta)`, where `msgSet` is a bare numeric literal in
a *local* id space (0-33, documented at `init_penaltyMsgGlyphs_8c04c35c` in that file) --
**not** an `INSTR_*` id. `msgSet` selects which HUD glyph banner is drawn
in-drive (`init_penaltyMsgGlyphs_8c04c35c[msgSet]`, glyph ids for `drawMsgGlyphRow_8c02b2f0`
in `02b2f0_drive_msg.c`), which is a third, unrelated id space again (glyph ids run
past 63). The `INSTR_*` dialog is shown only once, after the run: returning to
the practice lesson list (`01e27c_practice_menu.c`) queues it, with the run's
single worst `msgSet` (`var_worstPenaltyMsgSet_8c1bb8ec`) converted to an
`INSTR_*` id via `init_penaltyMsgSetInstr_8c045208[]` for the closing
instructor comment. **Story and free-run results
(`01d7fc_results.c`) never consult that table** -- those modes show only the
pass/fail badge, no per-offense commentary. So every `INSTR_*` penalty
message that fires at all is practice-mode-only; each `adjustPoints_8c02b464` call
site in `02b464_grading.c` now carries a `/* -> INSTR_* */` comment
naming which one it ends up showing there.

Four of the 32 defined penalties never appear as a value in
`init_penaltyMsgSetInstr_8c045208[]`, so as far as traced they are defined
but never actually raised: `INSTR_COLLISION_CAR_SEVERE`,
`INSTR_OFF_COURSE_MAJOR`, `INSTR_NO_SIGNAL_TURN`, and
`INSTR_BAD_STOP_POSITION_2` (several other `msgSet` ids collapse onto a
lower-severity `INSTR_*` than their delta would suggest, e.g. the worst
off-course case shows the same message as the medium one).

**Traffic offenses**

- **Collisions** -- with cars (minor/medium/severe/fatal) and with walls
  (minor/medium/severe). Also a near-miss-pedestrian warning.
- **Signals** -- pulling away without signalling, turning without signalling,
  changing lanes without signalling (`ILLEGAL_LANE_CHANGE`).
- **Traffic lights** -- running a red (`SIGNAL_VIOLATION`), and stopping past
  the line (`BAD_STOP_LINE`).
- **Speed and handling** -- speeding (minor/major), harsh acceleration
  (`RAPID_ACCEL`), hard braking, swerving.
- **Lane discipline** -- wrong lane, straddling lanes, wrong way, blocking an
  intersection.
- **Off course** -- leaving the route.

**Service errors**

- Pulling away with the doors open (`DOOR_OPERATION`).
- Failing to announce the next stop (`ANNOUNCEMENT`).
- Missing a stop entirely (`MISSED_STOP`), or stopping badly at one
  (`BAD_STOP_POSITION`) -- too far from the kerb or short of the mark.
- Running late (`TIME_MANAGEMENT`). The schedule is part of the job, so
  falling behind costs points like any other error.

**Railway crossings.** Graded, on the Ome route only. Its one crossing sits
between stops 2 and 3, on every Ome course. The rules come from road attribute bits
`0x30000000` (`ROAD_CROSSING_*`, `02b464_grading.c`), and only 4 polygons in
the game's attribute maps carry them, all at the `O_FUMI` gate posts
(~(7225, 6347)). `gradeFrame_8c02bcd8` enforces two rules:

- **Full stop in the approach** (zone 1, ~6.6m deep, ending at the gate
  line). Entering it arms the check, and leaving it before speed is exactly 0
  costs -30. Banner: 踏切前不停止 (no stop before the crossing).
- **No stopping on the tracks** (zone 2). A stationary frame with probe A
  or C (4.9m behind) in it costs -50 at once, then again every 0x78 frames.
  Banner: 踏切内停車 (stopped in the crossing).

Both map to `INSTR_BLANK`, an empty dialog sequence, so practice mode has
no instructor line for them. The scenery is separate:
`fumiCrossingTask_8c02a4f8`'s gate/train animation runs on a scene-script
trigger (top byte of `scenePresetIds_0x3bc`), not on the bus. Its type-6 row in
`init_8c03c504` sits after the table's `-1` terminator, so the animated gate
apparently never loads.

This also resolves a naming trap flagged during this investigation:
`BusState.crossingSearchDone_0x334`/`crossingSearchSide_0x338` looked like a
promising lead (their names, and the "second press of an already-latched
side" input pattern at `024280_bus_input.c:441-511`, suggested a "look both ways"
mechanic), but they are unrelated to the railway crossing. They drive
`BusDriveFindLaneTarget_8c023e7e` (`023938_bus_drive.c`) -- the search for a lane-change
target point via route-line-segment intersection (`IntersectSegments_8c0206f0`)
in auto drive mode (`var_driveMode_8c1bb8c8 != 0`), which steers along the route line.
"Crossing" there means a *route-line* crossing (two line segments
intersecting), not the railway *level* crossing -- a coincidental name
collision. Renamed to `laneTargetSearchDone_0x334`/`laneTargetSearchSide_0x338`
to remove the trap.

The score bar itself is `var_runState_8c2285c4.driverPoints_0x0c`, driven by
`02b464_grading.c`; it also feeds the end-of-run badge tier (see
`AWARD_TIER_*`).

## Stops and segments

- **A passenger stop = a CourseSegment boundary.** The loading screen at a
  stop is the SEGMENT_RELOAD state machine in `013ae8_route.c`; while it
  loads, the bus interior view is shown with passengers entering/leaving.
- All pedestrians and interior passengers are rendered as low-resolution
  billboards.
- During the interior scene, passengers move in a low-fps stop-motion manner;
  no texture animation.

**Which stops need servicing.** Confirmed: NOT every stop needs a stop, and
the decision is a per-segment *active-stop* flag (`var_segmentHasStop_8c2286a4[segment]`),
decided once at course load by `StopSetup_8c02caba` (`02c884_stop.c`)
before driving starts, not by a live "passenger requests a stop" event:
`CourseSegment.type_0x00 == 2` forces a stop there; a segment carrying a
matched story event (`EventScanCandidates_8c02b03c`) is also forced; the
remainder are filled randomly up to the course's
`[randomStopCountMin_0x14, randomStopCountMax_0x18)` total, skipping any
`type_0x00 == 3` segment (permanently excluded from random picking). A
segment's waiting passengers are spawned only when it is flagged active
(`pickWaitingPassengers_8c02c8ae`, gated on `var_segmentHasStop_8c2286a4`), so from the
player's side "someone is waiting" and "the stop is due" are the same
thing, even though the true cause is this precomputed flag, not a live
request. `StopAreaRecord.ukn_0x00` (a per-route physical stop-location
record: the course's `lineBus_0x08` table, `02c884_stop.h`) was checked as a
candidate for a live per-stop request flag; no reader or writer exists in
decompiled code, and unlike `var_segmentHasStop_8c2286a4` (a per-run scratch array reset
every course) it belongs to a table that reads as static per-route load
data, so it does not fit the request role. Its purpose is still unconfirmed.
- **World-space marker.** `drawStopMarker_8c02cd92`
  (`02c884_stop.c`) draws a `fuu.njd`/`fuu.pvm` model (its own animation
  loop) at the upcoming active stop's position/heading, but only while
  `StopUpdateArrival_8c02ce48`'s state machine is in its approach phase
  (`var_runState_8c2285c4.stopPhase_0x20 == 2`) -- i.e. only for a stop that was flagged
  active. This is very likely the world-space marker the player remembers;
  its actual on-screen color is not confirmed from code (no texture data
  decompiled here).
- **HUD indicator.** `drawHud_8c01fbac` (`01fa78_hud.c`) draws a sprite
  from `var_busStopTexlist_8c1bc424` at a fixed screen slot, blinking per
  `var_hudState_8c22643c.blinkTimer_0x18`'s timer, whose meaning depends on
  `var_runState_8c2285c4.stopPhase_0x20`: icon `0x1f` during phase 2 (approaching the next
  active stop) is shown unconditionally while blinking -- this is the HUD
  upcoming-stop indicator. Phase 1 (just departed) instead shows icon
  `0x1e`, but only while `var_hudState_8c22643c.driveMarkIcon_0x14 != -1`;
  phase 0/4 (cruising) draws that field itself, which is not a stop icon:
  `updateTask_8c01ff48` is its only write site outside
  `HudInstall_8c02018c`, latching `markDriveFlags_0x3b0`'s low 3 bits as a
  sprite id (`marker + 0x1f`). Those bits come from the attribute grid under
  the bus, so this is the map's own drive instruction -- not the driver's
  turn signal, as an earlier revision of this file claimed -- and the -1
  holds only until the bus crosses the first marked cell of the run. The HUD
  reuses one screen slot for that instruction (phase 0/4), a signal reminder
  (phase 1), and the next-stop indicator (phase 2) as the bus moves through
  the stop cycle.
- **The chime.** Two independent chime systems live in `driveCueTask_8c020214`
  (`020214_drive_cue.c`), operating on `DriveCueState var_driveCueState_8c2264b8` (`020214_drive_cue.h`):
  - `stopAnnounceState_0x08`/`stopAnnounceTimer_0x10` is the **driver's own
    stop announcement**, armed by `announceRequested_0x0c`, which
    `BusTask_8c022bdc` sets on the first A press of a drive. Its state
    machine plays a route-specific door-chime cue, then (after 60 frames) an
    actual spoken stop-name announcement via `SndPlayAdx_8c010cd6`. This is the
    mechanic the `ANNOUNCEMENT` penalty grades ("failing to announce the
    next stop"). Not purely player-initiated, though: `gradeFrame_8c02bcd8`
    (`02b464`) sets the same latch right after docking that penalty, so the
    announcement plays anyway once the game has charged you for missing it.
  - `ambientChimeState_0x00`/`ambientChimeTimer_0x04` is an unrelated ambient
    chime played periodically while driving (gated on a low
    `var_busState_8c1bb9d0.speed_0x27c` threshold, on a randomized repeat),
    not tied to stops at all.
  - `segmentChimeLatch_0x14` plays a short chime (case default in the
    function's tail block) but, despite its name, is gated on a *fixed*,
    hand-authored list of specific segment indices per route (e.g. Shinjuku:
    4, 5, 10, 21, 22, 23) -- not on `var_segmentHasStop_8c2286a4`, the actual per-run active-
    stop selection, and only while the camera is in a third-person mode
    (`var_cameraMode_8c227d9c >= 2`). Since the fixed list and the
    randomized active-stop set are unrelated, this chime cannot be a general
    "your requested stop is near" signal; it fires at the same handful of
    route locations every run regardless of which stops that run actually
    needs. What those specific locations are (landmarks? scripted
    narration cues?) is not yet traced -- flagging the name as
    possibly misleading rather than renaming it without more evidence.
  - No chime tied specifically to a stop being newly flagged
    active/requested (as opposed to the driver's own announcement, or the
    location-fixed cue above) was found in the decompiled code.

## Story event selection (`02af78_event`)

Which cutscene fires at a stop is resolved by `02af78_event`. Each route
(Shinjuku/Wangan/Ome) has an `EventEntry` table
(`init_shinjukuEvents_8c04b1f0`/`init_wanganEvents_8c04abb0`/`init_omeEvents_8c04b920`); an entry is eligible for a
segment if its time-of-day matches the current course, its packed
day-of-week mask contains the in-run day (`var_progress.days_0x00` -- so day-
of-month IS an input, confirming the prior guess), and its packed
prerequisite codes against progress flags pass. `EventScanCandidates_8c02b03c`
computes eligible entries once per course load;
`EventPickForSegment_8c02b170` narrows to the current segment and randomly
picks one, arming `var_cutsceneActive_8c1bb900`; `EventApplyFlags_8c02b292`
then applies the chosen entry's actions once the cutscene plays: persistent
progress flags, and/or a per-day "run flag" (`var_runEventFlags`) that lets an
event fired on an earlier segment gate a later segment's pick. Run flags are
cleared at the start of each scan and never saved. One scan == one course
attempt == one day: the day counter advances whether the course is passed or
failed (confirmed by play), so run flags are effectively per-day scratch state.
Practice mode and the course menu skip selection entirely.

## Known shipped bugs (preserved)

Real defects in the SHIPPED game, found while decompiling and deliberately
kept (correct for functional equivalence -- do not "fix" these). Each is
also noted in a code comment at its site; this is the index. Both are the
same shape: the address of a pointer variable (`&ptr`) passed where the
pointer's value (`ptr`) was meant.

- **`BusDriveFindLaneTarget_8c023e7e`** (`023938_bus_drive.c`) calls `sdMidiPlay` with
  `&var_midiHandles_8c0fcd28[0]` -- the array's address -- where every other
  call site in the codebase passes the handle value
  `var_midiHandles_8c0fcd28[0]`.
- **`CollisionFindTaskHit_8c02e400`** (`02e400_collision.c`) builds the "self"
  bounding box from `&init_variantBoxes_8c04c940[idx]` (the table slot's address) while a
  candidate's box comes from `init_variantBoxes_8c04c940[idx]` (the box it points at), so
  self is built from the pointer table's own bytes reinterpreted as floats.

**Was listed here, now resolved -- `(ResourceGroup *)&var_markTexlist_8c1bc418`
is correct.** `var_markTexlist_8c1bc418`, `var_markPartsDat_8c1bc41c` and
`var_markDat_8c1bc420` (`1ba1c8_globals.h`) are byte-contiguous and exactly
`sizeof(ResourceGroup)` -- and `requestVehicleAssets_8c013ae8` /
`GameInit_8c0134ec` fill them from `mark.pvm`, `mark_parts.dat` and `mark.dat`,
the same pvm/`_parts.dat`/`.dat` triple `GameInit_8c0134ec` loads into the
*declared* `ResourceGroup var_loadingResourceGroup_8c1bc3f8`'s
`tlist_0x00`/`tanim_0x04`/`contents_0x08`. So `tanim_0x04` reading
`var_markPartsDat_8c1bc41c` is what a real group does too, which was the last
doubt. Three globals named per-field before the struct existed, not a bug; the
same holds for the bus-stop trio at 8c1bc424. Folding each into one symbol is a
move-data job.


