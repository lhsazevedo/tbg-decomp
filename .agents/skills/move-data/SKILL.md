---
name: move-data
description: Moves global data ownership between pre-data asm files and unit files (C or decompiled asm), including exports/imports, section B allocations, and test visibility gating. Use when relocating variables between groups like 012f44/013ae8/0149b0, fixing undefined extern symbols after moves, or converting private globals to STATIC/AIFDEF UNIT_TESTING patterns.
---

# Move Data Ownership

Use this workflow when a variable currently lives in a shared pre-data file (for example `src/asm/014f54_text_pre_data.src`) and should be owned by a specific unit (`.c` and/or `src/asm/decompiled/*.src`).

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

## Conversation-Derived Conventions

- If a symbol is only used by the owner C file and tests, prefer:
  - C: `STATIC`
  - asm: `.AIFDEF UNIT_TESTING` export
- If non-owner asm unit references symbol (for example `0129cc.src`), keep it global/public.
- For grouped migrations (e.g. `; 013ae8`), move all group members together unless explicitly split.
