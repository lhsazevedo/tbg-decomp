# Ghidra MCP

A Ghidra MCP server is wired to this project. It exposes the live Ghidra project
under `ghidra/Tokyo Bus Guide/`, so you can read decompilations and maintain
names/types directly. Tools are deferred — fetch schemas with ToolSearch
(`select:mcp__ghidra__<name>` or keyword search) before calling.

## The program

- Current project is **`../tbg-ghidra`** ("Tokyo Bus Guide Decomp"), program
  **`flycast_ram.bin`** (a Flycast RAM dump; single `ram` space, `8c000000`-`8cffffff`).
  It replaced the corrupted `1ST_READ.BIN` project; older notes below still say
  `1ST_READ.BIN` but the live target is `flycast_ram.bin`.
- **Addresses are full `8c0xxxxx`** — same as the repo's `8c<addr>` suffixes.
  Range `8c010000`–`8c4fffff`. (image_base is 0, but functions/data live in the
  `8c...` block, so always use the full address.)
- ~2160 functions, ~13k symbols. Many are still `FUN_8c...` (unnamed) or carry
  old/inconsistent names.

## Gotchas

- `search_functions` results show a **leading `_`** (export artifact), e.g.
  `_task_title_8c015ab8`. `decompile_function` returns the clean name. Ignore the `_`.
- **Ghidra names lag the repo's conventions.** Old names use varied styles
  (`task_title_8c...`, `prob_task_8c...`, `FUN_pushDialogTask_8c...`). Repo
  convention is `Name_8c<addr>` / `<Desc>Task_8c<addr>` (see AGENTS.md). When
  maintaining, rename toward repo convention; treat existing Ghidra names as hints,
  not truth.
- Strings: many "strings" are literal hex address text (e.g. `"8c033380"`);
  `list_strings` filtering is picky and reports "no quality strings" easily.

## Common tools

- Read: `decompile_function`, `get_function_by_address`, `search_functions`,
  `list_data_items`, `get_xrefs_to` / `get_function_callers`.
- Maintain: `rename_function_by_address` (has `dry_run`), `rename_data`,
  `set_function_prototype`, `create_struct` / `apply_data_type`, `set_plate_comment`.
- Persist edits with `save_program` / `save_all_programs`.

## Bulk-syncing names from the build

`scripts/sync_ghidra_symbols.py` pushes every named symbol from the matching build
into the live project (functions + data, public + static; build wins). Run it after
naming a batch of things in the source tree to re-seed Ghidra.

- Needs the matching build current and assembled with `-debug` (embeds debug
  symbols in the .obj, which is where statics come from -- see
  docs/lessons_learned.md), and Ghidra open with the plugin on `127.0.0.1:8089`.
- Addresses are resolved map-free: each symbol name encodes its address, and the
  few that don't are placed from their object-section's anchor.
- Prints a summary by default; pass `--apply` to write. Save the program afterward.
