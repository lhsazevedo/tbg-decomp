---
name: decompile-function
description: Incrementally decompile one SH4 function (or block) into C with mirroring unit tests. Use when porting a function from src/asm/<addr>.src to src/<addr>.c, adding tests under tests/<unit>/, or continuing a partially-decompiled translation unit.
---

# Decompile a Function

Port one function (or one block of a large function) from asm to C, test it, repeat until the whole translation unit (TU) is done.

## Inputs

- **Ghidra via MCP** -- the primary source. `mcp__ghidra__decompile_function` for the
  pseudocode; see `docs/ghidra-mcp.md` for the wider toolset. Ghidra is enough to
  understand a function at a high level and write the first test. Ghidra's own symbol
  DB may still show **pre-rename** names (`var_8c...` where the project has since
  renamed to `var_pauseActive_8c...`) -- translate using the project's current symbol,
  not Ghidra's stale label.
- `src/asm/decompiled/<addr>.src` -- original asm. **Authoritative** for constants,
  table bases/bounds, side-effect ordering, and branch structure, but reading it is a
  **last resort**: the dual-object test (below) proves equivalence far more cheaply
  than a careful asm read. Reach for it only when a test disagrees between the two
  objects and Ghidra doesn't explain why -- or to settle a single fact (a literal-pool
  constant, which register a field access hangs off) that Ghidra garbled.

## Why you can lean on the test instead of the asm

Each TU group in `tests.php` lists **both** objects (`<unit>_src.obj` and
`<unit>_c.obj`). The suite runs every test against each, so a passing file proves the
original asm and your C behave identically. That safety net means you can work from a
Ghidra-level understanding and a hypothesis, and let the run catch a misread -- you do
**not** need to fully reverse the asm up front.

## Working style: guess and validate, don't get paralyzed

The dual-object test is your instinct check -- **use it instead of exhaustive study.**

- **Gather just enough, then write.** For a big function, get enough for the signature
  and one branch -- don't try to understand the whole thing, or every symbol and unit it
  touches, before writing anything. Chasing the ownership of every callee up front is
  the main way to stall.
- **Take a guess and let the run tell you.** Trust the Ghidra decompile (through the
  FPSCR/pointer noise), write the C and a test, and run it against `<unit>_src.obj`
  first. If your guessed calls/args match the original asm, both objects pass and your
  instinct was right; if not, the failure names the exact discrepancy. This is cheaper
  than reverse-engineering to certainty.
- **One branch at a time** for large functions: signature + skeleton, test a branch,
  add the next branch, test again. Or write the whole function on a confident
  Ghidra-trust guess and cover every branch in one test file -- either way, validate
  against the original asm.
- A referenced callee that lives in a not-yet-decompiled unit only needs a **minimal
  header** declaring the one symbol you call (see AGENTS.md) -- you do not need to
  understand or decompile that unit to call into it.

## Procedure

1. **Analyze in Ghidra.** Decompile the target via MCP and read it at a high level:
   what does it take, what does it touch, what does it write? Enough to name it and
   write a test -- don't overthink it. Magic numbers are fine to start with; mapping
   them to named SDK macros (grep `shinobi/include/`) is a **secondary** pass.
2. **Rename the asm to the chosen name.** Both objects must export the *same* symbol
   for the dual-object test to target it. `sed` the export directive, the label, and
   every internal callsite in `src/asm/decompiled/<addr>.src` from `_FUN_8c<addr>` to
   `_<name>_8c<addr>` (leave plain-text comment mentions alone).
3. **Write the C** in `src/<addr>.c`. Match asm store ordering, not Ghidra's (Ghidra
   reorders writes and invents bounds). Keep each binary function as its own C
   function -- do **not** factor shared helpers between siblings; it breaks matching.
4. **Declare public functions** in the unit's `src/<addr>.h` (create it if absent).
   `STATIC` functions need no header entry. If the header carried a placeholder
   `extern TaskAction <name>;` (function-pointer *variable*) for a not-yet-decompiled
   callback, change it to a real prototype (`void <name>();`, or `(Task *task)` if it
   reads the task) once you give it a body -- SHC enforces same-TU type consistency and
   errors `2136 (E) Type mismatch` otherwise. Use empty parens `()` for a `TaskAction`
   callback that ignores its `(task, state)` args (precedent: `PspTask_8c012324`);
   name the param only if the body uses it. The install site is untyped (`void *`), so
   the arity doesn't have to match there.
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

## Test harness (sh4objtest DSL)

- **Ordered expectation queue.** `call()`, `shouldCall()`, `shouldWrite*()` all push
  onto one ordered list consumed in that exact order. Put `$this->call(...)` first,
  then every expectation in real execution order (writes and calls interleaved).
- **`setSize()` is for external/stub symbols only** (reserves a fake address, size 4
  for a function). Calling it on a symbol **defined in the same object** (a sibling
  function, or a symbol `addressOf()` already auto-allocated) throws "already defined"
  / "already allocated". Mock a same-object function with `shouldCall()->andReturn()`
  directly -- no `setSize()`.
- **`addressOf('_sym')` auto-allocates** -- don't also `setSize()` the same symbol.
- **Float args pass as PHP floats** in `->with(...)` (e.g. `->with($grp, 0x7b, 0.0,
  0.0, -1.1)`); a plain string literal matches a `char*` arg by content.
- **Void functions need no `shouldReturn()`.** A struct-pointer param is an
  `alloc(size)` seeded with `initUint*` at field offsets, passed via `call()->with()`.

## Gotchas

- Uninitialized memory is **randomized** by default, so a read-modify-write (e.g.
  `x |= mask`) reads garbage. Seed every field the function reads with `initUint*`
  (or `doNotRandomizeMemory()`), even ones that start at 0.
- `src/` and `tests/` are Shift-JIS: **ASCII only** (no `->` arrows, smart quotes).
- C macros are invisible to PHP tests -- use raw hex with a comment naming the macro.
- Loop bounds: asm usually computes `base + size` inline, so the end symbol need not
  be pinned. Only `rellocate()` an **imported** bound symbol; never a local one.
- Coverage counts padding bytes and won't reach 100% on a fully-tested unit -- don't
  chase it.
- The decompiled function stays in `src/asm/decompiled/<addr>.src`: the matching build
  keeps assembling the original asm until the unit's C is proven byte-matching.
