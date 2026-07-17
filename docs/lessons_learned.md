# Lessons Learned

Indexed, non-obvious findings from decompiling this codebase. Each entry is a
mistake made once, or a quirk of the toolchain/original build, worth not
re-discovering. Add an entry whenever you hit something surprising; keep each
one short and dated with the unit where it was found.

## Index

- [Struct fields can be separately-imported symbols in asm](#struct-fields-can-be-separately-imported-symbols-in-asm)
- [Calls to sibling functions in the same TU are still mocked](#calls-to-sibling-functions-in-the-same-tu-are-still-mocked)
- [A nested single-statement `if(cond){break;}` can compile to unreachable bytes](#a-nested-single-statement-ifcondbreak-can-compile-to-unreachable-bytes)
- [`asmsh -debug` embeds symbols in the .obj; `-debug=d` writes a side .DWF](#asmsh--debug-embeds-symbols-in-the-obj--debugd-writes-a-side-dwf)
- [Marking known-dead asm lines with coverage tags](#marking-known-dead-asm-lines-with-coverage-tags)
- [Renaming an exported symbol: unit tests won't catch a missed caller](#renaming-an-exported-symbol-unit-tests-wont-catch-a-missed-caller)
- [Strength-reduced loops and nested ifs fold back to idiomatic C](#strength-reduced-loops-and-nested-ifs-fold-back-to-idiomatic-c)

## Struct fields can be separately-imported symbols in asm

**Found in:** `02af78_event` (2026-07-12)

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

## Calls to sibling functions in the same TU are still mocked

**Found in:** `02af78_event` (2026-07-12)

A function that calls another function decompiled earlier *in the same
`.c`/`.src` file* (e.g. `scanUnlockCandidates_8c02b03c` calling
`hasProgressFlag_8c02afbe`, both in `02af78_event.c`) does **not** execute the
callee for real during a unit test, even though both end up in the same
object file. The test harness still intercepts the `BSR`/call and requires
an explicit `shouldCall(...)->andReturn(...)` expectation, exactly as for a
genuinely external/cross-TU call. Omitting it fails with `Unexpected
function call to _<callee>`.

**Fix:** always mock intra-TU calls with `shouldCall()`/`andReturn()` rather
than seeding the callee's real backing memory and expecting it to run for
real -- this is simpler anyway and keeps the test focused on the function
under test.

## A nested single-statement `if(cond){break;}` can compile to unreachable bytes

**Found in:** `02af78_event` (2026-07-12)

Pattern:
```c
if (mode_matches) {
    if (some_call(...) != 0) {
        break;
    }
} else if (other_mode && other_call(...) == 0) {
    break;
}
```
For the *first* arm only, this compiler (SHC, 1997) folds the outer and inner
condition into one instruction that branches straight past the loop body to
the post-loop code -- but it still emits the literal `break;`'s own bytes
(an unconditional branch to the same target) immediately after, as dead
filler that no control-flow edge ever reaches. `sh4objtest --coverage` then
reports that line as permanently uncovered no matter what the test does,
because it genuinely never executes -- confirmed via `sh4objtest inspect
--format=json`'s `debugLines` (per-line address ranges) plus `-d` trace: the
branch for the enclosing `if` jumps directly over the `break;`'s address
range.

**Fix (best, in C):** split the `if/else if` into two independent top-level
`if`s, ending the first arm with an explicit `continue;` instead of letting
it fall out of the `if`:
```c
if (mode_matches) {
    if (some_call(...) != 0) {
        break;
    }
    continue;
}
if (other_mode && other_call(...) == 0) {
    break;
}
```
This isn't just cosmetic -- it changes codegen. With the `else if` chain, the
compiler has to jump *past* the second condition entirely when the first
arm's outer test is true, and that extra unconditional jump is what leaves
the dead `break;` bytes behind. With two independent `if`s, the first arm's
only fallthrough target *is* `continue`'s target, so the compiler folds the
`break;`'s jump-out-of-loop directly into the same conditional branch that
tests `some_call(...) != 0` -- confirmed via `sh4objtest inspect
--format=json`'s `debugLines`: the `break;` line has no address range of its
own at all (zero-width), rather than a range coverage reports as dead.
Result: no coverage gap, no exclusion tag needed, and the shape (guard
clause + independent checks) is arguably clearer than the original nested
`if/else if` anyway.

**Fix (when the `continue` restructuring doesn't apply, e.g. no loop to
`continue` in, or sh4objtest < v0.1.35):** rewrite the first arm to match
the second arm's shape -- combine the two conditions with `&&` into a single
`if (mode_matches && some_call(...) != 0) { break; }` instead of nesting.
Same behavior, one direct conditional branch, no leftover dead bytes.

**Fix (last resort):** tag the dead `break;` with `coverage:ignore-next-line`
(see the entry below, requires sh4objtest >= v0.1.35) and leave the
`if/else if` shape as-is. Only reach for this when neither restructuring
above is a natural fit -- an exclusion tag hides a genuine dead-code
artifact instead of removing it, so a real fix is always preferable when one
exists.

The dead-bytes pattern itself also shows up verbatim in the *original*
`.src` asm (built by the same-era SHC compiler -- confirmed present in
`02af78_event.src`'s `_scanUnlockCandidates_8c02b03c`/`_pickUnlockCandidate_8c02b170`).
There, restructuring is never an option regardless of sh4objtest version --
the archived asm must stay byte-identical to the real game binary -- so the
coverage tags below are the only fix.

## Marking known-dead asm lines with coverage tags

**Found in:** `02af78_event` (2026-07-12), sh4objtest v0.1.35+

`sh4objtest suite --coverage` supports source-line exclusion tags, matched as
plain substrings so they work in both `//`/`/* */` (C) and `;` (asm)
comments:

- `coverage:disable` / `coverage:enable` -- exclude an inclusive block
- `coverage:ignore-next-line` -- exclude only the following line

They require `tests.php` to declare a `sourcePaths` map from the Wine debug
path prefix to a host directory relative to `tests.php`'s own location, e.g.
`'sourcePaths' => ['Z:\\app\\src' => 'src']`. Without it the tags are
silently ignored (`ignoredLinesFor()` returns nothing when the debug path
doesn't resolve).

**Use case:** original `.src` asm legitimately contains code that's
unreachable in practice and that we must not rewrite (see the entry above,
and negative-index-handling branches from `CMP/PZ`-guarded shift/modulo
codegen that no real caller ever triggers, and one-off alignment `.RES.W`
padding words in a literal pool, which aren't code at all). Tag these with a
short comment explaining *why* they're unreachable instead of leaving
`--coverage` to report them as gaps indistinguishable from a genuine missing
test -- also requires bumping `docker/Dockerfile`'s sh4objtest version and
rebuilding the local image (`docker build -t lhsazevedo/tbg-decomp docker/`)
to pick up a version that supports the tags -- `docker-run.sh`/`docker-shell.sh`
use a prebuilt image tag, so editing the Dockerfile alone does nothing until
it's rebuilt.

## Renaming an exported symbol: unit tests won't catch a missed caller

**Found in:** `02af78_event` (2026-07-12)

When you rename a `.EXPORT`ed function/variable, the rename has to reach
*every* caller across the whole tree, not just the owning unit -- and a
green test suite does **not** prove you got them all. Two structural blind
spots in the harness hide a missed caller:

- Each unit test loads only its own object(s), and cross-unit calls are
  mocked (see the sibling-mock entry), so no test ever links the renamed
  symbol against a stale importer in another unit.
- A single-file compile of the renamed unit succeeds too -- nothing it
  builds references the old name.

The miss only surfaces at the **full `make` link** as
`105 UNDEFINED EXTERNAL SYMBOL(<unit>._<oldname>)`, because still-undecompiled
asm units keep `.IMPORT`ing the old name.

**Fix:** blanket-sed the old name across the whole tree in one shot --
`find src tests -type f -exec sed -i 's/<old>/<new>/g' {} +` -- rather than
grepping first and sed'ing a filtered list. There's no need to know *where*
the callers are up front; a single recursive pass covers `.c`/`.h`, *all*
`.src` (callers hold it as `.IMPORT` + `.DATA.L _oldname` literal-pool
entries), `tests/` mocks, and comments together. Omit the leading `_` so
`foo`->`bar` also catches the asm `_foo`. Then run the full `make` link (not
just `run_tests.sh`): that link, via UNDEFINED EXTERNAL SYMBOL, is the only
check that proves the sed reached every caller -- it's the verification step,
not a pre-sed grep.

## Strength-reduced loops and nested ifs fold back to idiomatic C

**Found in:** `02af78_event` (2026-07-12)

Because the goal is functional equivalence -- and the exact original
structure stays archived byte-for-byte in `.src` -- the decompiled C is free
to take a more idiomatic shape than the Ghidra/asm output, and usually
should. Reshaping isn't just cosmetic: it changes codegen, and can remove
compiler dead-code artifacts (see the break-bytes entry). Two recurring
foldbacks:

- **Pointer-walk + separate trailing counter -> plain indexed loop.** A loop
  that advances a struct pointer (`entry++` in the `for` step) while a
  *second* variable counts (`index++` at the bottom of the body, used only to
  index a parallel array) is the compiler's strength-reduction of one indexed
  loop. When `index == entry - base` holds every iteration, drop the walking
  pointer: `for (index = 0; base[index]...; index++)` with a local
  `entry = &base[index]`. One variable instead of two kept in lockstep, and
  `continue` guards then work correctly (they still reach the loop's own
  `index++`, which a hand-placed bottom-of-body `index++` would skip).

- **Nested `if`/`else if` chains -> guard clauses with early
  `continue`/`return`.** Invert per-iteration `if (match) { body }` nests into
  `if (!match) continue;` + flat body, split an `if/else if` pair into two
  independent `if`s where the first ends in `continue`, and duplicate a
  trivial tail write (e.g. `cutsceneActive = 0`) across early-return exits
  rather than nesting to share it. Flatter, and -- per the break-bytes entry
  -- the independent-`if`+`continue` form is what actually deletes the dead
  `break;` bytes.

Both are behavior-identical; prove it with the dual-object test plus
`--coverage` (100% on both objects, no new uncovered ranges).

## `asmsh -debug` embeds symbols in the .obj; `-debug=d` writes a side .DWF

`asmsh`'s `-debug` puts debug info **inside the .obj**; `-debug=d` writes it to a
separate `.DWF` and leaves the .obj without it. So `sh4objtest inspect
--format=json` only reports `debugSymbols` (asm emits type `Label`, `shc` emits
`Func`/`Var`) under plain `-debug`. Those debug symbols are the only place
**statics** appear -- `exports` lists just the public symbols. `Makefile.matching`
uses `-debug` for this reason; `scripts/sync_ghidra_symbols.py` depends on it.

Real symbols carry a leading `_`; internal auto-labels (`LAB_`, `LP_GEN_`, the
section symbol `P`) do not -- a clean filter.
