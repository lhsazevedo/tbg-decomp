---
name: decompile-function
description: Incrementally decompile one SH4 function (or block) into C with mirroring unit tests. Use when porting a function from src/asm/<addr>.src to src/<addr>.c, adding tests under tests/<unit>/, or continuing a partially-decompiled translation unit.
--- 

# Decompile a Function

Port one function, or one block of a large function, from asm to C. Test each part as you go until the whole translation unit (TU) is done.

## Avoid these two failure modes

### 1. Do not wait until you fully understand the function

Start writing and testing as soon as you know the signature and enough to attempt one path. A wrong C implementation that fails quickly is more useful than spending a long time trying to understand everything first.

Get a body into `src/<addr>.c` and run a test against it as early as possible. Use failures to correct your understanding.

### 2. Do not read the raw asm

Do not open `src/asm/decompiled/<addr>.src` to understand the function. Do not use a Ghidra tool to get the same thing instead, for example `disassemble_function`, `get_assembly_context`, `get_function_pcode`, `disassemble_bytes`, or `search_instructions` on the whole function. `decompile_function` is fine; raw disassembly, from any source, is not.

Reading raw disassembly is slow and can cause you to repeat the same mistaken interpretation in both the C code and its tests. Use the dual-object tests instead.

If you believe reading raw disassembly is necessary, ask the user for permission first. Explain why you need it and exactly what you want to inspect. This applies no matter which tool would supply it.

The one exception is step 5: while debugging a failing test, `--disasm` may show you the disassembly of instructions the test actually executed.

The normal loop is:

**fix Ghidra's types -> inject its output verbatim -> comment it all out -> re-enable and test one path at a time**

## Inputs

* **Ghidra via MCP** is the main and normally the only source.

  * Use `mcp__ghidra__decompile_function` for pseudocode.
  * See `docs/ghidra-mcp.md` for the other tools.
  * Ghidra may still use old symbol names such as `var_8c...` after the project has renamed them to names such as `var_pauseActive_8c...`. Use the project's current symbol names.
  * Do not use a Ghidra tool for raw disassembly (`disassemble_function`, `get_assembly_context`, `get_function_pcode`, `disassemble_bytes`, `search_instructions`, etc.) as a stand-in for reading `src/asm/decompiled/<addr>.src`. Same rule, same permission requirement: see "Do not read the raw asm" below.
* Do not use `src/asm/decompiled/<addr>.src` as an input.

## Why testing is enough

Each TU group in `tests.php` contains both the unit's asm object (`build/output_test/src/asm/decompiled/<unit>.obj`) and its C object (`build/output_test/src/<unit>.obj`). Every test runs against both objects.

If the same test passes against both, the original asm and your C behave the same for that test. Use failed tests to learn the behavior instead of trying to understand everything before writing code.

## Working style

* **Gather only enough information to start.** The signature and one path are enough. Do not investigate every callee first.
* **If uncertain, guess and test.** A passing test confirms the guess for that case; a failing test shows the mismatch.
* **Prefer running a test to spending several minutes reasoning.**
* **For large functions, work one path at a time.** Do not uncomment the entire function at once.
* If a callee belongs to a unit that has not been decompiled yet, add only the minimal header declaration needed for the symbol you call, as described in `AGENTS.md`.

## Procedure

### 1. Fix Ghidra's types and decompile again

Decompile the target and briefly inspect its parameters, reads, and writes.

Correct obvious type problems before translating. This step is only for improving Ghidra's output, not for fully understanding the function. If you start investigating types mainly to understand behavior, stop and continue to step 3.

Fix things such as:

* Symbols already typed in the project but still shown raw in Ghidra. Use tools such as:

  * `mcp__ghidra__apply_data_type`
  * `mcp__ghidra__set_decompiler_variable_type`
  * `set_function_prototype`
* Contiguous `DAT_XXXXXXXX` values that the project treats as one struct or array. Create and apply the struct using tools such as `mcp__ghidra__create_struct` and `add_struct_field`, so accesses become fields instead of separate globals.
* Callee prototypes with missing or incorrect arguments that make call sites inaccurate.

Then run `force_decompile` and inspect the result again.

If the result still has serious problems, such as invented arguments, badly broken control flow, or floats represented as integers throughout, tell the user instead of translating unreliable output.

You may leave magic numbers as-is for now. Looking up named SDK macros in `shinobi/include/` is a later cleanup step.

### 2. Rename the asm symbol

The original and C objects must export the same symbol so the dual-object test can target both.

Use `sed` to change the export directive, label, and internal call sites in `src/asm/decompiled/<addr>.src` from:

`_FUN_8c<addr>`

to:

`_<name>_8c<addr>`

Do not change plain-text mentions in comments.

This is a mechanical edit only. Do not read the file to understand the asm.

### 3. Inject the Ghidra body verbatim

Write a small temporary script that takes the Ghidra decompiler output and inserts it into the new function body in `src/<addr>.c`.

Do not retype the body manually. Do not clean it up, reinterpret it, or fix suspected mistakes while inserting it. Preserve the Ghidra output verbatim at this stage.

### 4. Comment out the entire body

Comment out every line of the injected body so the function exists and the file compiles, but the body is inactive.

You will re-enable it one tested path at a time.

### 5. Pick the first path and test it

Pick the first path to test.

Create its test in:

`tests/<unit>/<addr>_name.php`

Use one `call()` per test and assert side effects in their exact execution order.

Register new test files in the TU's group in `tests.php`. When adding the first function for a unit, create the group and include both the asm and C objects.

Run the individual test file:

```bash
./docker-run.sh ./scripts/run_tests.sh -c /app/tests/<unit>/<file>.php | tail -40
```

At this point, use the original asm object to establish the expected behavior because the C path is still commented out.

When a test fails:

* Always use `| tail`; the useful failure information is usually near the end.
* If the cause is unclear, rerun with `-v --disasm` and inspect the fulfilled expectations:

```bash
./docker-run.sh ./scripts/run_tests.sh -c /app/tests/<unit>/<file>.php -v --disasm \
  | grep 'Fulfilled:'
```

The last fulfilled expectation shows where your expected behavior stopped matching the function.

Keep adjusting the test until it passes against the asm object. If a failure is unclear, simplify the test, remove an expectation, or split the path into smaller pieces and rerun it.

Do not open the `.src` file. You may inspect only the disassembly printed by `--disasm` for instructions actually executed by the test.

### 6. Enable that path and make the C pass

Uncomment only the code for the path you just tested. Leave all untested paths commented out.

Split commented regions when necessary, including shared tails or an `else` body that is not covered yet.

Edit the C until the test passes against both objects.

Do not assume Ghidra preserved the correct write order or bounds. The ordered test expectations determine the actual required behavior.

Interpret failures this way:

* If the test passes for `.src` but fails for C, fix the C.
* If it starts failing for `.src`, your expected behavior is wrong. Return to step 5 and correct the test.

### 7. Repeat

Repeat steps 5 and 6 for each remaining path until:

* no code remains commented out; and
* every branch is tested, including negative and empty cases.

Keep each path small enough to test quickly. If one path is taking too long, split it into smaller paths.

### 8. Finish the unit

* Declare public functions in the unit's `src/<addr>.h`, creating the file if necessary.

* Do not add `STATIC` functions to the header.

* If the header contains a placeholder such as:

  `extern TaskAction <name>;`

  for a callback that previously had no body, replace it with a real function prototype once the function is implemented.

  Use:

  `void <name>();`

  or:

  `void <name>(Task *task);`

  if the body reads the task.

  SHC requires consistent types within the TU and otherwise reports `2136 (E) Type mismatch`.

  For a `TaskAction` callback that ignores its `(task, state)` arguments, use empty parentheses `()`, following `inputMenuTask_8c012324`. Only name the parameter when the function uses it.

  The installation site uses `void *`, so the arity does not need to match there.

* Keep every binary function as a separate C function. Do not extract shared helpers between sibling functions because that breaks matching.

* Commit with only a title, for example:

  `Decompile FUN_8c012718`

* Do not run the full `./build.py` until the whole TU has been decompiled.

## Lessons learned

Read `docs/lessons_learned.md` for toolchain and asm issues already discovered in other units.

Add a new entry when you encounter a new non-obvious issue.

## Test harness: `sh4objtest` DSL

* `call()`, `shouldCall()`, and `shouldWrite*()` all add entries to one ordered expectation queue. Put `$this->call(...)` first, followed by every expected write and call in actual execution order.

* Use `setSize()` only for external or stub symbols. A function uses size `4`.

* Do not call `setSize()` for a symbol defined in the same object, including sibling functions or symbols already allocated by `addressOf()`. Doing so produces an "already defined" or "already allocated" error.

* To mock a same-object function, use `shouldCall()->andReturn()` directly without `setSize()`.

* `addressOf('_sym')` allocates the symbol automatically, so do not also call `setSize()` for it.

* Pass float arguments to `->with(...)` as PHP floats, for example:

  `->with($grp, 0x7b, 0.0, 0.0, -1.1)`

* A normal string literal can match a `char *` argument by content.

* Void functions do not need `shouldReturn()`.

* For a struct-pointer parameter, use `alloc(size)`, initialize fields with `initUint*` at their offsets, and pass the allocation through `call()->with()`.

## Gotchas

* Uninitialized memory is randomized by default. If the function reads a field before writing it, including read-modify-write operations such as `x |= mask`, initialize that field with `initUint*` or use `doNotRandomizeMemory()`. Initialize fields even when their expected starting value is `0`.
* `src/` and `tests/` use Shift-JIS. Use ASCII only. Do not use Unicode arrows, smart quotes, or similar characters.
* C macros are not visible to PHP tests. Use the raw hexadecimal value and add a comment naming the macro.
* For loop bounds, asm often computes `base + size` directly, so the end symbol usually does not need to be pinned. Only `rellocate()` an imported bound symbol; never use it for a local bound.
* Coverage includes padding bytes, so a fully tested unit may still report less than 100% coverage. Do not try to eliminate that difference.
* Keep the decompiled function in `src/asm/decompiled/<addr>.src`. The matching build continues assembling the original asm until the unit's C implementation has been proven byte-matching.
