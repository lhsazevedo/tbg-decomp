# Gameplay Reference

How the game actually plays, from someone who has played it. Useful context
when naming functions/data: the code was written to implement *this*.

Tokyo Bus Guide (1999, Dreamcast): a Japanese bus simulator. You drive a city
bus along a predefined route with predefined passenger stop requests.

## Modes

- **Main modes** (3 routes x 3 times of day = 9 courses; see
  `reference_route_assets` / `013ae8_route_load.c` for the courseId scheme):
  - **Story mode**: at some stops, after the interior view, a cutscene event
    may trigger showing passenger dialog: pre-rendered slideshows with audio
    and textboxes. Seen events unlock passenger profiles in the event menu
    ("PROFILE FILE" notebook UI -- grid of portraits, per-character bio pages
    with a checkbox grid of seen events). This is NOT `01d290_album`: the
    album is a separate screen showing a few pictures unlocked throughout
    story mode. The event menu's owning unit is not yet identified.
  - **Free run**: same courses, no cutscenes/events.
- **Practice**: 11 runs teaching individual mechanics, each preceded by a
  slideshow guide. Runs take place on sections (segments) of the main route
  maps. Hypothesis: these are the extra courseTable entries 27+
  (`init_course27..47_`) -- unconfirmed.

## Driving loop

- Follow driving rules: speed limit, turn signals, traffic lights, smooth
  acceleration/braking, and the time schedule.
- Committing a penalty displays a text message on screen.
- A single **score bar** drains with penalties; gameplay ends if it empties.
  Passing bus stops (both boarding stops and drive-bys) replenishes it a bit.
- Reaching the final stop ends the run: score screen, then back to course
  select.
- The VMU LCD shows art during gameplay (see `01bb48` `vmsLcd_*`).

## Stops and segments

- **A passenger stop = a CourseSegment boundary.** The loading screen at a
  stop is the SEGMENT_RELOAD state machine in `013ae8_route_load.c`; while it
  loads, the bus interior view is shown with passengers entering/leaving.
- All pedestrians and interior passengers are rendered as low-resolution
  billboards.
- During the interior scene, passengers move in a low-fps stop-motion manner;
  no texture animation.

## Story event selection (`02af78`)

Which cutscene fires at a stop is resolved by `02af78`. Each route
(Shinjuku/Wangan/Ome) has an `EventEntry` table
(`init_8c04b1f0`/`init_8c04abb0`/`init_8c04b920`); an entry is eligible for a
segment if its time-of-day matches the current course, its packed
day-of-week mask contains the in-run day (`var_progress.days_0x00` -- so day-
of-month IS an input, confirming the prior guess), and its packed
prerequisite codes against progress flags pass. `scanEventCandidates_8c02b03c`
computes eligible entries once per course load;
`pickSegmentEvent_8c02b170` narrows to the current segment and randomly
picks one, arming `var_cutsceneActive_8c1bb900`; `applyEventFlags_8c02b292`
then applies the chosen entry's post-conditions (sets progress/unlock flags
or a same-run "already fired" flag) once the cutscene plays. Practice mode
and the course menu skip selection entirely.

## Open questions

- Practice runs = courseId 27+ is a player recollection, not verified in
  code.
