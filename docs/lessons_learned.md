# Lessons Learned

Indexed, non-obvious findings from decompiling this codebase. Each entry is a
mistake made once, or a quirk of the toolchain/original build, worth not
re-discovering. Add an entry whenever you hit something surprising; keep each
one short and dated with the unit where it was found.

## Index

- [Struct fields can be separately-imported symbols in asm](#struct-fields-can-be-separately-imported-symbols-in-asm)

## Struct fields can be separately-imported symbols in asm

**Found in:** `02af78` (2026-07-12)

A field we already model as part of a C struct (e.g. `PlayerProgress.field_0x04`
within `var_progress_8c1ba1cc`) can show up in some *other* unit's asm as its
own separately-`.IMPORT`ed symbol (e.g. `_var_8c1ba1d0`) that happens to sit at
that struct's field address — visible because the owning unit's `.src` exports
both the base symbol and the field-offset symbol at contiguous addresses.

If you leave the separate import as-is in a decompiled-and-tested unit, the C
object (which addresses the field via the struct) and the src object (which
addresses it via the standalone symbol) both resolve to the right address in
the real build, but the **unit test** must satisfy both — normally caught as
`Trying to read from unresolved relocation _var_<addr>` when only the struct's
base symbol was sized/allocated.

**Fix:** don't work around this in the test with `rellocate()`. Instead, treat
the standalone import as the wrongly-disassembled artifact and correct the
asm: drop the `.IMPORT`, and rewrite every use (typically a `.DATA.L` literal
pool entry) as `_baseSymbol+H'<offset>` sum-expression addressing instead of
the separate name. This keeps one C struct as the single source of truth and
avoids the test-side alias entirely. The owning unit's now-unreferenced
`.EXPORT` line for the dropped symbol can be left alone if that unit isn't
decompiled yet — it's harmless dead export, not worth touching out of scope.
