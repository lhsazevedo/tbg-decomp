---
name: move-data
description: Moves global data ownership between pre-data asm files and unit files (C or decompiled asm), including exports/imports, section B allocations, and test visibility gating. Use when relocating variables between groups like 012f44_game/013ae8/0149b0, fixing undefined extern symbols after moves, or converting private globals to STATIC/AIFDEF UNIT_TESTING patterns.
---

# Move Data Ownership

Use this workflow when a variable currently lives in a shared pre-data file (for example `src/asm/014f54_text_pre_data.src`) and should be owned by a specific unit (`.c` and/or `src/asm/decompiled/*.src`).

## Automated: `.src` -> `.src` moves

For the asm-to-asm case (temp file -> owning unit's `src/asm/decompiled/*.src`), use
`scripts/move_data.py` instead of hand-editing:

```
scripts/move_data.py --src <temp.src> --dest <owner.src> --from <_sym> \
    [--to <_sym>] [--after <_sym> | --before <_sym>] [--dry-run]
```

It moves the labeled block(s) for a single symbol or a file-order-inclusive span
(`--from`..`--to`), handles sections C/D/B, creates a missing section in the dest,
and recomputes `.IMPORT`/`.EXPORT` in both files (`imports == referenced - defined`;
the moved symbol's export travels to the dest). Shift-JIS in `;.SDATA` comments is
byte-preserved. It warns if a moved block -- or the block the cut leaves exposed in
the origin -- starts at a non-4-aligned address, which signals a wrong span boundary
/ mis-decompiled object. (A misaligned section *end* is fine; the linker realigns.)

By default the block lands at the end of the dest's matching section; use
`--after`/`--before` to place it next to an existing dest symbol (needed to keep
link output byte-identical when the owner already has data in that section).
Either way, confirm with `scripts/dcdiff.py` + the matching build after a move.

The manual steps below still apply to the **C-file / test half** of an ownership
move (`STATIC` gating, headers, `tests/*` fixups), which the script does not touch.

## Goals

- Keep one clear owner for each symbol.
- Keep export/import declarations consistent with ownership.
- Keep BSS (`.SECTION B`) definitions in the owner unit.
- Preserve testability (`STATIC` in C, `.AIFDEF UNIT_TESTING` in asm) for private symbols.
- Avoid linker regressions (`UNDEFINED EXTERNAL SYMBOL`).

## Procedure

1. Identify the symbols and target owner.
2. Classify each symbol as:
   - **Public**: referenced by other units.
   - **Private**: only used in owner unit.
3. Move declarations and BSS definitions from old owner to new owner.
4. Update C visibility (`STATIC` vs global) and asm export gating (`.AIFDEF UNIT_TESTING`) based on usage.
5. Verify no stale `.IMPORT` remains in the owner asm file.
6. Recheck for unresolved externs and fix visibility if needed.

## Symbol Migration Rules

### 1) From old pre-data owner (`*_pre_data.src`)

- Remove moved symbols from header exports.
- Remove moved symbol allocations from `.SECTION B,DATA,ALIGN=4`.
- Keep remaining groups intact and in order.

### 2) To target decompiled asm owner (`src/asm/decompiled/<unit>.src` or unit asm)

- For symbols now owned by this file:
  - Replace `.IMPORT _symbol` with `.EXPORT _symbol` if present.
  - If symbol is not in import list, add `.EXPORT _symbol` in sorted export region.
- Add/keep `.SECTION B,DATA,ALIGN=4` allocations in this file with exact original sizes.
- For symbols that should be test-only exports, put exports in:
  - `.AIFDEF UNIT_TESTING`
  - `.AENDI`

### 3) To target C owner (`src/<unit>.c`)

- Convert `extern` declarations into definitions in owner C file.
- Use `STATIC` (uppercase macro) for private globals.
- Keep globals non-`STATIC` if referenced by other units (link-visible).
- Prefer using shared `STATIC` macro from common header (currently `serial_debug.h`).

## Visibility Policy

- **Private symbol** (used only in owner unit):
  - C: `STATIC type symbol;`
  - asm: export only under `.AIFDEF UNIT_TESTING` if tests need symbol access.
- **Public symbol** (used by other units):
  - C: non-static definition.
  - asm: normal `.EXPORT`.

## Sorting and Placement

- Keep symbol lists sorted by address/name style already used in file.
- Do not leave `.EXPORT` inside an import block by mistake.
- Keep related group comments accurate (`; 013ae8`, `; 0149b0_sbinit`, etc.).

## Verification Checklist

- [ ] Symbol removed from old owner exports.
- [ ] Symbol removed from old owner `.SECTION B`.
- [ ] Symbol defined in new owner (`.c` and/or `.src`) with correct size.
- [ ] Owner asm has `.EXPORT`, not `.IMPORT`, for owned symbols.
- [ ] C visibility matches usage (`STATIC` vs global).
- [ ] Test-only symbols are gated with `.AIFDEF UNIT_TESTING` when appropriate.
- [ ] No unresolved extern errors remain for moved symbols.

## Common Failure Modes

- **Undefined external after move**: symbol made `STATIC` but still referenced elsewhere; make it global.
- **Duplicate definition**: symbol left in both old and new BSS owners; remove old allocation.
- **Wrong section ownership**: symbol moved in export list but BSS allocation not moved.
- **Export placement drift**: new `.EXPORT` added in imports region; relocate to export block.
- **Orphaned section C data after extracting a const block**: if you extract a `.SECTION C, DATA, ALIGN=4` header together with its const block, the remaining data in the source file loses its section declaration and the assembler silently drops it. Always re-insert `.SECTION C, DATA, ALIGN=4` before the first remaining label in the source file, OR extract only the DATA lines and leave the section header behind.

## Moving Section C Consts to a Unit

When a unit's `init_*` data array in section D references `_const_8cXXXXXX` symbols from `sectionC.src`, those consts can be moved into the unit's `.src` file verbatim so ownership is consolidated. Procedure:

1. **Extract the const block** from `sectionC.src` ‚Ä? copy the `.SECTION C, DATA, ALIGN=4` line plus the data labels/bytes verbatim.
2. **Keep the section header in `sectionC.src`** ‚Ä? after removing the const data lines, ensure a `.SECTION C, DATA, ALIGN=4` directive remains before the first surviving data label. Failure to do this causes the assembler to silently drop all remaining section C data, shifting every subsequent const address and breaking the matching build.
3. **Add the block to the unit `.src`** ‚Ä? append it before `.END`, after the section D data. Switching back and forth between section C and section D in a single `.src` file is fine; the assembler merges same-named sections.
4. **Update exports in the unit `.src`** ‚Ä? add `.EXPORT _const_8cXXXXXX` for each moved const (remove the corresponding `.IMPORT`; local symbols need no import).
5. **Remove exports from `sectionC.src`** ‚Ä? delete the `.EXPORT` lines for the moved consts.
6. **C file** ‚Ä? use direct SJIS string literals (same style as MenuDialog sequences in `course_menu.c`). The compiler puts anonymous strings in section C of the unit's obj; they do not conflict with the named `_const_*` labels in the matching build because those are separate symbols.

### Why the binary stays identical
The section C layout in the linked binary is determined by link order. As long as:
- the unit `.src` is linked at the same relative position (before `sectionC.src`), and
- the const block is appended AFTER the unit's existing section C data (so order within the unit obj is preserved),
the byte sequence in the final section C is identical to before ‚Ä? just split across different obj files.

## Fixing Tests After Moving Initialized Data

After moving a symbol from `*_pre_data.src` / `sectionD.src` to a TU (section D of its `.c` or `.src`), the corresponding tests need updating:

### setSize removal
- `setSize('_sym', n)` allocates an extern symbol. Once the TU defines it, this call fails with "Cannot allocate symbol X, it is already defined in the object file".
- **Fix**: remove the `setSize()` call entirely.

### initUint32 / initUint32Array on TU-owned section D data does NOT work
- TU-defined section D data is loaded into test memory AFTER `initUint32`/`initUint32Array` writes. The TU initialization silently overwrites any test-level init.
- **Fix**: remove the `initUint32Array()` call for the moved symbol. Either accept the TU-defined values as-is, or re-examine whether overriding is truly needed.
- Exception: BSS symbols (allocated via `setSize`) CAN be initialized with `initUint32` since BSS is zero-filled and test writes are not overwritten.

### String address expectations when data moves to TU
- Each test file runs twice: once against `_src.obj` (asm stub), once against `_c.obj` (C compiled). The TU-owned section C/D string addresses differ between the two objects (`_const_8cXXXXXX` asm labels vs. anonymous C compiler strings).
- Address-based expectations (e.g. `->with($this->addressOf('_const_8c037f60'))`) only work for one obj variant.
- **Fix**: use PHP string literals in `with()` ‚Ä? the framework compares by C string content: `->with("TOKYOBUS.001")`. This works for both obj variants.

### Loop count changes when array sentinel moves
- If a test previously overrode a TU-owned array with fewer entries (e.g. 4 entries ending with `""`) to bound a loop, removing that override means the loop now runs to the TU-defined sentinel (e.g. 11 entries). Update expected call counts accordingly.
- Example: `_init_saveNames_8c044d50` ‚Ä? old tests used 4 entries with `""` sentinel; TU has 10 names + `""`. Tests that expected 3 `_buIsExistFile` not-found calls for "saving possible" now need 10.

### vmuStatusMessages / pointer arrays used as message selectors
- If a function indexes into a TU-owned `char*` array to pass to another function, and tests previously overrode that array with fake addresses (0xcafe000x), those overrides no longer work.
- **Fix**: remove the `initUint32Array` override; update `->with(0xcafe000x)` expectations to use the actual SJIS string content: `->with("„Çª„Éº„ÉñÂèØËÉΩ„Åß„Å?")`.

## Conversation-Derived Conventions

- If a symbol is only used by the owner C file and tests, prefer:
  - C: `STATIC`
  - asm: `.AIFDEF UNIT_TESTING` export
- If non-owner asm unit references symbol (for example `0129cc.src`), keep it global/public.
- For grouped migrations (e.g. `; 013ae8`), move all group members together unless explicitly split.
