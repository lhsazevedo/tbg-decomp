---
name: decompile-function
description: Incrementally decompile one SH4 function (or block) into C with mirroring unit tests. Use when porting a function from src/asm/<addr>.src to src/<addr>.c, adding tests under tests/<unit>/, or continuing a partially-decompiled translation unit.
---

# Decompile a Function

Port one function (or one block of a large function) from asm to C, test it, repeat until the whole translation unit (TU) is done.

## Inputs

- **Ghidra via MCP** — the primary source. `mcp__ghidra__decompile_function` for the
  pseudocode; see `docs/ghidra-mcp.md` for the wider toolset. Ghidra is enough to
  understand a function at a high level and write the first test.
- `src/asm/decompiled/<addr>.src` — original asm. **Authoritative** for constants,
  table bases/bounds, side-effect ordering, and branch structure, but reading it is a
  **last resort**: the dual-object test (below) proves equivalence far more cheaply
  than a careful asm read. Reach for it only when a test disagrees between the two
  objects and Ghidra doesn't explain why.

## Why you can lean on the test instead of the asm

Each TU group in `tests.php` lists **both** objects (`<unit>_src.obj` and
`<unit>_c.obj`). The suite runs every test against each, so a passing file proves the
original asm and your C behave identically. That safety net means you can work from a
Ghidra-level understanding and a hypothesis, and let the run catch a misread — you do
**not** need to fully reverse the asm up front.

## Procedure

1. **Analyze in Ghidra.** Decompile the target via MCP and read it at a high level:
   what does it take, what does it touch, what does it write? Enough to name it and
   write a test — don't overthink it. Magic numbers are fine to start with; mapping
   them to named SDK macros (grep `shinobi/include/`) is a **secondary** pass.
2. **Rename the asm to the chosen name.** Both objects must export the *same* symbol
   for the dual-object test to target it. `sed` the export directive, the label, and
   every internal callsite in `src/asm/decompiled/<addr>.src` from `_FUN_8c<addr>` to
   `_<name>_8c<addr>` (leave plain-text comment mentions alone).
3. **Write the C** in `src/<addr>.c`. Match asm store ordering, not Ghidra's (Ghidra
   reorders writes and invents bounds). Keep each binary function as its own C
   function — do **not** factor shared helpers between siblings; it breaks matching.
4. **Declare public functions** in the unit's `src/<addr>.h` (create it if absent).
   `STATIC` functions need no header entry.
5. **Write/extend tests** in `tests/<unit>/<addr>_name.php` (one `call()` per test).
   Assert side effects in exact asm execution order. Cover every branch, including
   negative/empty arms.
6. **Register** new test files in the TU's group in `tests.php` (create the group,
   listing both `_src.obj` and `_c.obj`, on the unit's first function).
7. **Run the single file**: `./docker-run.sh ./scripts/run_tests.sh -c
   /app/tests/<unit>/<file>.php` (add `-d` for a trace). Both objects must pass.
8. **Commit** title-only (e.g. `Decompile FUN_8c012718`). Defer full `make` until the
   whole TU is decompiled.

## When logic is complex or unclear

Test the **original assembly first**, before writing any C. Because the asm is already
one of the two objects, you can write tests and run them against `<unit>_src.obj`
alone to pin the behavioral contract; then decompile the C to make the same tests
pass. This avoids encoding a misread into both C and tests at once.

## Lessons learned

Check `docs/lessons_learned.md` for non-obvious toolchain/asm quirks already
hit in past units, and add an entry there when you hit a new one.

## Gotchas

- Uninitialized memory is **randomized** by default, so a read-modify-write (e.g.
  `x |= mask`) reads garbage. Seed every field the function reads with `initUint*`
  (or `doNotRandomizeMemory()`), even ones that start at 0.
- `src/` and `tests/` are Shift-JIS: **ASCII only** (no `->` arrows, smart quotes).
- C macros are invisible to PHP tests — use raw hex with a comment naming the macro.
- Loop bounds: asm usually computes `base + size` inline, so the end symbol need not
  be pinned. Only `rellocate()` an **imported** bound symbol; never a local one.
- Coverage counts padding bytes and won't reach 100% on a fully-tested unit — don't
  chase it.
- The decompiled function stays in `src/asm/decompiled/<addr>.src`: the matching build
  keeps assembling the original asm until the unit's C is proven byte-matching.
