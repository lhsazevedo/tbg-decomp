# Serial Debug Logging

`LOG_*` macros from `serial_debug.h` prove the decompiled C (not the asm) runs,
and make flow visible on the serial console. Active only in the full build
(`SERIAL_DEBUG`); compiled out under `UNIT_TESTING`, so they never touch test
expectations. Include `serial_debug.h`.

Double-paren syntax, ASCII only (`src/` is Shift-JIS), `\n`-terminated,
side-effect-free arguments (they don't run under tests).

## Message shape

Prefix every message with an uppercase module tag: `[TITLE]`, `[INPUT]`,
`[ASSET_QUEUES]`, `[MAIN_MENU]`. One tag per unit; reuse the unit's existing one.

Two shapes:

- **State machines** (per-frame task with a state field): `[TAG] State changed: NAME`.
  Log the state by **name**, never a raw number. Use a `SERIAL_DEBUG`-guarded
  `DEBUG_<module>StateNames[]` and a `CHANGE_STATE(x)` macro that assigns the
  field and logs in one step (see `019e98_main_menu.c`). Log only the transition,
  not every frame; the same target name repeating from different sources is fine.

- **Everything else**: `[TAG] funcName_8caddr: what happened`. Name the function
  so the line is direct proof it ran. Include relevant values by name/string
  (`%s`, `%d`, `%p`), e.g. `pushInputTask_8c0128cc: queueing input handler (%s)`.

## Levels (by frequency)

- `TRACE` - per-frame / hot dispatch branches worth tracing. Terse; floods easily.
- `DEBUG` - transitions and meaningful one-off events (state change, task install,
  enqueue, remap). The common case.
- `INFO` - one-time lifecycle (module init/teardown, queue init/reset).
- `WARN`/`ERROR`/`FATAL` - anomalies.
