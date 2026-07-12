# Ghidra MCP

A Ghidra MCP server is wired to this project. It exposes the live Ghidra project
under `ghidra/Tokyo Bus Guide/`, so you can read decompilations and maintain
names/types directly. Tools are deferred — fetch schemas with ToolSearch
(`select:mcp__ghidra__<name>` or keyword search) before calling.

## The program

- Target program is **`1ST_READ.BIN`** (the game binary; `ip.bin` also exists but
  is usually closed). Pass `program="1ST_READ.BIN"` if ever ambiguous.
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
