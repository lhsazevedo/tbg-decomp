---
name: add-serial-logging
description: Add serial debug logging (LOG_TRACE/DEBUG/INFO/WARN/ERROR/FATAL from serial_debug.h) to a decompiled C unit, so the running code can be observed and proven to be the decompiled one in the emulator. Use when asked to add/instrument logging, trace flow, or verify which build is running.
---

# Add Serial Debug Logging

Instrument a decompiled `src/<addr>.c` unit with `LOG_*` calls. Logging proves the
decompiled C (not the original asm) is what runs in the emulator, and makes the
per-frame / state flow visible on the serial console.

## How the macros work

From `serial_debug.h`:

- Levels (high to low frequency of importance): `LOG_FATAL`, `LOG_ERROR`,
  `LOG_WARN`, `LOG_INFO`, `LOG_DEBUG`, `LOG_TRACE`.
- **Double-parenthesis syntax** (the macro takes a single token and expands to
  `serialprintf x`):
  ```c
  LOG_DEBUG(("[TAG] message\n"));
  LOG_DEBUG(("[TAG] value=%d name=\"%s\"\n", n, str));
  ```
- `serialprintf` supports printf-style `%d`, `%x`, `%s`, etc.
- A unit must `#include "serial_debug.h"` to use them (it also provides `STATIC`).

## Build gating (why this is safe for tests)

| Build | `.sub` file | Defines | `LOG_*` |
|---|---|---|---|
| Unit tests | `build/shc_testing.sub` | `UNIT_TESTING` (no `SERIAL_DEBUG`) | compile to **nothing** |
| Full game | `build/shc.sub` | `SERIAL_DEBUG` (no `UNIT_TESTING`) | **active** |

So logs never affect unit-test expectations, and are live only in the real
build. `DEBUG_LEVEL` defaults to `LOG_LEVEL_TRACE` (everything prints) unless
overridden, so `TRACE` lines show by default but can be filtered by lowering it.

## Procedure

1. Add `#include "serial_debug.h"` if not already present.
2. Place a `LOG_*` call at each meaningful point (see level guide below).
3. Build tests for the unit (`run_tests.sh -c ...`) -> must still pass unchanged
   (logs compiled out).
4. Run full `make` -> the unit must compile with **0 warnings**; `serialprintf`
   resolves because `shc.sub` defines `SERIAL_DEBUG`.
5. Commit title-only (e.g. `Add serial debug logging to <unit> ...`).

## Choosing a level (by call frequency)

- **Per-frame / hot paths** (task actions run every frame, dispatchers): `LOG_TRACE`.
  Keep these terse; they can flood the console, and the user can quiet them via
  `DEBUG_LEVEL`.
- **Transitional / rare-but-meaningful events** (task installed, state changed,
  name set, soft-reset combo, mode remap): `LOG_DEBUG`.
- **One-time lifecycle** (module init/teardown): `LOG_INFO`.
- **Anomalies**: `LOG_WARN` / `LOG_ERROR` / `LOG_FATAL`.

## Message conventions

- Prefix with an uppercase module tag in brackets: `[INPUT]`, `[TITLE]`,
  `[COURSE_MENU]`, `[TXT]`, `[ASSET_QUEUES]`. Reuse the unit's existing tag if it
  has one; otherwise pick one and use it consistently across the file.
- **Name the function** in the message (e.g. `[INPUT] FUN_8c012970: dispatch
  task_8c012504`). Seeing that line is direct proof that specific decompiled
  function ran -- the main reason for logging.
- End each message with `\n`.
- **ASCII only** (`src/` is Shift-JIS): plain `"` quotes, `->` not arrows, no
  smart punctuation.

## Critical gotcha: log arguments must be side-effect-free

Arguments are evaluated **only when `SERIAL_DEBUG` is defined** (i.e. not in unit
tests). An expression with a side effect inside a `LOG_*` call would run in the
real build but not under tests -- a behavioral divergence the tests can't catch.
Only read variables / call pure accessors in log arguments; never increment,
assign, or call a mutating function there.

## Verification checklist

- [ ] `#include "serial_debug.h"` present.
- [ ] All existing unit tests for the unit still pass (logs compiled out).
- [ ] Full `make` compiles the unit with 0 errors / 0 warnings.
- [ ] No side effects inside any `LOG_*` argument.
- [ ] Per-frame paths use `TRACE`; transitions use `DEBUG`/`INFO`.
- [ ] Messages are ASCII, `\n`-terminated, tagged, and name the function.
