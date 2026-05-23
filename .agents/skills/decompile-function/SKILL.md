---
name: decompile-function
description: Incrementally decompile one SH4 function (or block) into C with mirroring unit tests. Use when porting a function from src/asm/<addr>.src to src/<addr>.c, adding tests under tests/<unit>/, or continuing a partially-decompiled translation unit.
---

# Decompile a Function

Port one function (or one block of a large function) from asm to C, test it, repeat until the whole translation unit (TU) is done.

## Inputs

- `src/asm/<addr>.src` — original asm. **Authoritative** for constants, table bases/bounds, side-effect ordering, and branch structure.
- `tests/<unit>/<name>.ghidra.c` — Ghidra reference. Useful for shape, but it **reorders writes and invents bounds** — verify every constant and the order of stores against the asm.

## Procedure

1. **Read the asm and the ghidra .c.** Magic numbers are fine to start with; mapping them to named SDK macros (e.g. `0x100` -> `PDD_DEVTYPE_VIBRATION`, grep `shinobi/include/` or ask) is a **secondary** pass, not a blocker.
2. **Write the C** in `src/<addr>.c`. Match asm store ordering, not Ghidra's. Keep each binary function as its own C function — do **not** factor shared helpers between sibling functions; it breaks matching.
3. **Write/extend tests** in `tests/<unit>/<addr>_name.php` (one `call()` per test). Assert side effects in exact asm execution order. Cover every branch, including negative/empty arms (they harden behavior even when they add no coverage %).
4. **Register** new test files in the TU's group in `tests.php`.
5. **Run the single file**: `./docker-run.sh ./scripts/run_tests.sh -c /app/tests/<unit>/<file>.php` (add `-d` for a trace).
6. **Commit** title-only (e.g. `Decompile FUN_8c012718`), including the `.ghidra.c` reference file. Defer full `make` until the whole TU is decompiled.

## When logic is complex or unclear

Test the **original assembly first**, before writing any C. Tests against the asm pin the behavioral contract; then decompile the C to make the same tests pass. This avoids encoding a misread of the asm into both C and tests.

## Gotchas

- `src/` and `tests/` are Shift-JIS: **ASCII only** (no `->` arrows, smart quotes).
- C macros are invisible to PHP tests — use raw hex with a comment naming the macro.
- Loop bounds: asm usually computes `base + size` inline, so the end symbol need not be pinned. Only `rellocate()` an **imported** bound symbol; never a locally-defined one.
- Coverage counts padding bytes and won't reach 100% on a fully-tested unit — don't chase it.
- The TU group in `tests.php` lists **both** objects (`<unit>_src.obj` and `<unit>_c.obj`). The suite runs against each, so the test list executes twice per file — the original asm and the decompiled C must both pass, proving behavioural equivalence. The decompiled function therefore stays in `src/asm/<addr>.src`.
