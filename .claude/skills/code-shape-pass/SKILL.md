---
name: code-shape-pass
description: Rewrite decompiler-shaped C into the way this codebase writes C, one function at a time, behavior unchanged and tests green after every function. Use when asked to clean up, deslop, or idiomize the code (not the comments) of an already-decompiled unit.
---

# Code shape pass

An AI-decompiled unit is usually correct and still reads like Ghidra output: pointer
walks instead of indexes, casts compensating for a wrong global type, spill temporaries,
gotos. This pass rewrites that into the idiom the rest of the codebase uses, one function
at a time, without changing what the code does.

This pass edits code. That makes it different from `unit-pass`, which cannot break a
build: here the unit's tests are not a final sanity check, they are the thing that makes
each step safe. Run them per function, not per unit.

Comments are out of scope -- `unit-pass` owns those.

## Before you start

1. **Check the matching build.** If the unit has no archived `src/asm/decompiled/<addr>.src`,
   stop and ask: that unit is compiled from C into the byte-matching build, and
   reshaping its code can move codegen off the match. As of this writing that is only
   `010080_main`, `010e90`, `014934`, `0149b0_sbinit`, `014a9c_tasks`, `014b8c_backup`,
   `016108`, `235ca0_nj_buffers` -- every other unit builds from the
   archived asm and only owes functional equivalence.
2. **Get a green baseline.** `./scripts/run_tests.sh -c /app/tests/<unit>/<file>.php` for
   the unit, plus `./scripts/check_data_match.sh` if the unit has a data section. A pass
   that starts red cannot tell its own breakage from the pre-existing kind.
3. **Have the asm open.** `src/asm/decompiled/<addr>.src` is the authority on what the
   function does. When C and asm disagree about what a reshape preserves, the asm wins.

## The worklist

One row per function in the unit, in file order, in a temp file in your scratchpad:

    id | function | verdict | tells found

Verdicts: `clean` (nothing to do), `reshaped`, `skipped` (with the reason), `frozen`
(still asm, not yet decompiled). Fill one row at a time, in order, and record the verdict
before moving on. A verdict covers exactly one function. The pass is done when every row
has a verdict, not when the file reads well.

## The loop, per function

1. Read the function, and the asm beside it if the shape is load-bearing.
2. Note which tells from the catalog it has.
3. Make the change.
4. Run that unit's test file. It must pass before you start the next row.
5. Record the verdict and move on.

Never batch edits across several functions and test at the end. When something goes red
you need it to be obvious which change did it, and with a batch it is not.

## Catalog of tells

**Pointer-walk loop -> plain index.** The decompiler emits cursor arithmetic where the
original reads as an indexed loop. Precedent: `TileStreamTeardown_8c021724`.

    for (p = &tbl[0]; p < &tbl[16]; p++) { p->flags = 0; }
    ->
    for (i = 0; i < 16; i++) { tbl[i].flags = 0; }

**Countdown loop that counts up in the asm** (or the reverse). Follow the asm.

**Goto to a shared epilogue -> if/else.** The asm jumps because it has no `else`; the C
does. A goto that only skips to common tail code is an if/else in disguise.

**Cast idiom compensating for a wrong type.** `*(Uint8 *)&var_award_8c1bb8f8` all over a
function means the global is typed wrong. Fix the declaration and the casts disappear.
Prefer that over preserving the cast: one type fix removes many tells.

**Phantom parameter from a merged float pool.** Ghidra sometimes unfolds PC-relative float
literal loads into an extra int argument that does not exist. Read the `LP_GEN_*` pool in
the asm to confirm before deleting a parameter.

**Spill temporaries.** A variable assigned once, used once, and named like `uVar3` is the
decompiler's register allocator leaking into the source.

**Redundant reinterpretation.** `(int)` round-trips through a float, `&` on something
already a pointer, a temporary that exists only to hold a cast.

## Out of scope

- **Renaming.** Naming is a separate job with its own conventions
  (`check_naming.py`).
- **Struct and data layout**, except a type correction that removes casts at its uses.
- **New helpers, split functions, deduplication, "while I'm here" improvements.** The
  output is the same function written the way this codebase writes functions.
- **Anything that changes behavior.** Functional equivalence is the project's goal; this
  pass never trades it for readability.

## Stop conditions

- **A test fails and the fix is to change the test.** Stop and revert the change. The test
  encodes what the asm does; a test edited to accommodate a reshape has deleted the only
  evidence the reshape was wrong.
- **A reshape cannot be made behavior-identical** (uninitialized reads, signed/unsigned
  edges, evaluation order the asm depends on). Leave the code as it is and record
  `skipped` with the reason -- those quirks are usually documented in a comment already.
- **The function is bug-for-bug faithful to an original-game bug.** Reshaping around it
  risks quietly fixing it. Leave it.

## Report

Verdict counts from the worklist, the units' test results, and any `skipped` rows with
their reasons. Say plainly if a function was left worse than you found it.
