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
- [Ghidra's SSA variable reuse can hide two different real values](#ghidras-ssa-variable-reuse-can-hide-two-different-real-values)

## Struct fields can be separately-imported symbols in asm

**Found in:** `02af78_event` (2026-07-12)

A field we already model as part of a C struct (e.g. `PlayerProgress.eventProgressFlags_0x04`
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
avoids the test-side alias entirely. Once no unit imports the label any more,
drop it from its defining `.src` too, rolling its `.RES.B` into the struct's own
symbol; the eleven `PlayerProgress` labels went that way, as the 24 `BusState`
ones did before them.

## Calls to sibling functions in the same TU are still mocked

**Found in:** `02af78_event` (2026-07-12)

A function that calls another function decompiled earlier *in the same
`.c`/`.src` file* (e.g. `EventScanCandidates_8c02b03c` calling
`hasEventProgressFlag_8c02afbe`, both in `02af78_event.c`) does **not** execute the
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
`02af78_event.src`'s `_EventScanCandidates_8c02b03c`/`_EventPickForSegment_8c02b170`).
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

A further reason not to scope the sed from a grep: in this shell `grep` is
aliased to `ugrep`, whose `\b` doesn't fire next to a leading underscore, so
`\b_FUN_8c...\b` matches nothing in `.src` labels or PHP `'_FUN_...'` string
literals. The filtered list comes back empty and the rename looks done. Use
`command grep` when you do need to search for one.

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

## After a public rename in a decompiled `.src`, rebuild the matching object before `check_naming`

`check_naming.py` cross-checks a decompiled unit's `.c` object against its
**matching** object (`build/output_matching/.../<unit>.obj`), which is assembled
from the archived `src/asm/decompiled/<unit>.src`. A `./scripts/run_tests.sh -c`
run only refreshes `build/output_test/` objects, so after `sed`-renaming a symbol
in the `.src` the matching object stays stale and the check reports
`symbol at <addr> differs: .c has '<New>', .src has 'FUN_<addr>' -- rename both`
even though both source files already agree. Run `make -f Makefile.matching` to
refresh it, then re-check. Only **public** (exported) renames trip this; a
`STATIC` rename is invisible in the C object so the cross-check skips it.

## Compute-then-store: a conditional overwrite of a global emits two writes

The asm for a value that's conditionally refined usually computes it fully in a
register and stores the global **once** at the end (e.g. `var = base` vs
`var = base + (page-1)*stride`, then a single `MOV.L Rn,@global`). Translating that
as `global = a; if (c) global = b;` compiles to **two** stores to the global. The
matching build (which uses the asm) is unaffected, but the dual-object test compares
the memory-write trace, so the extra `global = a` write shows up as an
`Unexpected write value ... to _global` failure on the C object only (the `.src`
object passes). Fix: compute into a **local**, store the global once. Only globals
are trace-checked, so intermediate writes to the local are free.

## A Ghidra global that's really `base_symbol + offset` computed inline

Ghidra sometimes invents a standalone global (`var_exp_8c1ba25c`) for what the
asm actually computes as arithmetic on a different symbol's address
(`var_progress_8c1ba1cc + 0x90`, i.e. a struct field) rather than a direct
relocation. A dual-object test using `setSize()`/`addressOf()` on the invented
name gets its own independent fake address, so the asm object write lands at a
totally different (and seemingly nonsensical) address than expected --
`Unexpected write address 0x... Expecting ... to _invented_name(0x...)`. When
that happens, read the relevant asm block directly: if the base register comes
from `MOV.L @(disp,PC),Rn` loading a *different* imported symbol followed by
`ADD #offset,Rn`, the "global" is actually `otherSymbol + offset` -- express it
as a struct field access in C and in the test (`addressOf('_other') + offset`),
not a separate `setSize()`. Note this can be genuinely per-caller: another unit
that imports the invented name **directly** (`.IMPORT _var_exp_8c1ba25c`) is
using a real, separate linker symbol that only *coincides* with the struct
field's address because sectionB.src labels it there. Check that caller's own
asm before touching it -- but a label that lands inside a struct we already
model is an alias either way, and folding every user of it is the end state.

A second shape of the same trap: when the base register holds a
compile-time-constant address (e.g. `r14` = `&var_busState_8c1bb9d0`), Ghidra
folds `r14 + 0x340` straight into the absolute literal `0x8c1bbd10` and shows it
as its own global. Several unrelated-looking globals a few hundred bytes apart,
all inside one struct's extent, are the tell -- they are just
`field_0x340`/`field_0x35c`/`field_0x378`. Found in `BusInitStart_8c023610`
(023310_bus_init).

## A same-object `const` used in a new test needs `.EXPORT` in the archived `.src` too

A `STATIC const` array already sitting in the C file (message box text, etc.)
compiles and links fine even when the matching label in
`src/asm/decompiled/<addr>.src` isn't `.EXPORT`ed -- nothing needs it *until*
a test's `addressOf('_const_...')` tries to resolve it against the asm object.
Without the export, `addressOf()` can't find the real symbol there and falls
back to an auto-allocated placeholder address, which happens to coincide
across unrelated tests (each fresh test VM allocates in the same order), so
every affected test fails identically with something like
`Unexpected argument for _swapMessageBoxFor_8c02aefc ... Expected 0x80034c,
got 0x...` -- the C object passes throughout, only the asm object mismatches, and
every failing call reports the *same* "Expected" address regardless of which
constant it actually is. Fix: add the label under the unit's existing
`.AIFDEF UNIT_TESTING` / `.AENDI` `.EXPORT` block in the `.src` file (mechanical
`sed`/`Edit`, not a read of the asm body).

## `muls.w` (0x200f) was missing from sh4objtest's simulator until this session

Compiler-emitted `(short)x * 100` lowers to `MULS.W`, not `MUL.L`; sh4objtest's
`Simulator.php` only had `MUL.L` (`0x0007`) and threw `Unknown instruction`
for `0x200f` on both objects identically (a real tooling gap, not a C bug).
Fixed upstream in the sh4objtest repo (mirrors the `MUL.L` case, sign-extends
the 16-bit operands into `macl`) and the `tbg-decomp` Docker image rebuilt.

## Ghidra can label a raw address inside another struct as its own fresh global

`TileStreamInit_8c02175a` (`02171c`) reads a 5-pointer array Ghidra decompiled
as a standalone global `var_8c1bb8a4`. The address `0x8c1bb8a4` is actually
`var_currentCourse_8c1bb868 + 0x3c`, i.e. `tileLayers_0x3c[0]` inside the
existing `CurrentCourse` struct -- Ghidra has no notion of the struct field
here, since the instruction loads a plain absolute address from the literal
pool, so it just names that address like any other global. A dual-object test caught it:
`setSize`-ing a fake `var_8c1bb8a4` gave the C object a plausible mock address
that happened not to match the asm object's real relocation target, so only
the `.src` side failed with an unrelated-looking garbage value. Fixed by
indexing into the real struct (`var_currentCourse_8c1bb868.tileLayers_0x3c[i]`)
instead of declaring a new global, and the alias label has since been folded out
of `sectionB.src`. Worth checking whenever a Ghidra global's address falls
inside an already-known struct's range.

## Every call to an exported symbol is intercepted, even same-object ones

sh4objtest's `matchBranch` treats *any* branch whose target resolves to a
named symbol as a call that must be in the expectation queue -- it doesn't
distinguish "external, must be mocked" from "same-object, could just run for
real". So once a `STATIC` helper is exported under `.AIFDEF UNIT_TESTING`
(required to test it directly), every caller's test must also add a
`shouldCall()` entry for it, even calls you'd rather let execute for real.
`TileStreamLoad_8c021810`'s test mocks `lookupTile_8c0217de` outright instead
(already covered by its own test file), which is simpler than trying to let
it "pass through" -- there's no such option in the DSL.

For stack-local out-params (e.g. `njReadBinary`'s `&fpos`/`&rtype`) whose
address you can't predict, don't reach for `WildcardArgument` or hardcoded
per-object addresses without checking first: run the test with a guessed
placeholder, read the real value off the `Unexpected argument ... got 0x...`
error (or `-v --disasm`), and hardcode that. It's usually the same address
for both objects; branch on `$this->objectFile` (see
`tests/016d2c_course_menu/8c0170c6_FUN_pushDialogTask.php` for the pattern)
only if it isn't.

## Ghidra drops a called SDK function's float argument entirely if it has no prototype

`trafficSignalTask_8c028258` (`028258`) calls `njSqrt`, but Ghidra decompiled it
as `njSqrt()` with zero arguments -- the real SH4 float argument register (FR4)
was left untracked because the imported `njSqrt` symbol had no prototype
(`param_count: 0` via `get_function_signature`). Worse, once the function's own
prototype was force-recomputed, Ghidra "fixed" the mismatch by inventing a
phantom incoming float parameter on the *caller* instead, shared identically
between two logically distinct branches -- a strictly worse, actively
misleading result. Fixed by setting `njSqrt`'s prototype explicitly
(`mcp__ghidra__set_function_prototype` -> `float njSqrt(float n)`) before
decompiling the caller. Worth checking whenever a call to a no-body/imported
SDK function shows no arguments in Ghidra's output.

## Mock addressOf() for two adjacent external symbols doesn't preserve their real relative offset

`FUN_8c02890c`/`FUN_8c028958` (`028258`) each zero-fill one of two adjacent
64-entry arrays (`var_8c227e2c`, `var_8c22802c`) that are external to the
unit under test (defined in `sectionB.src`). The compiler folded
`FUN_8c028958`'s loop base into `var_8c227e2c + 0x200` instead of relocating
`var_8c22802c` directly -- plausible since the two symbols sit back-to-back
in the same section. `addressOf()` on an external symbol allocates it via
the test's own bump allocator, independent of any other symbol, so two
separately-`addressOf()`'d adjacent globals land at unrelated mock
addresses and the offset-folded reference resolves somewhere unexpected.
Fix: compute the second address explicitly from the first
(`addressOf('_var_8c227e2c') + 0x200`) and pin it with
`rellocate('_var_8c22802c', ...)` before calling `addressOf('_var_8c22802c')`,
so both the offset-folded asm reference and the C object's direct symbol
reference agree.

## `_quick_odd_mvn` is a struct copy, and Ghidra renders it as a bare no-arg call

SHC compiles a small struct assignment (e.g. `NJS_POINT3 a = b;`) into a call to
the runtime helper `_quick_odd_mvn`, which takes **dest in R1, src in R2, byte
count in R0** -- not the R4-R7 default. Ghidra has no body for it, so it shows up
as `_quick_odd_mvn();` with no arguments, which is easy to skim past and drop
entirely: `FUN_8c02845a` (`028258`) had two such calls copying the entry's two
position vectors into the task state, and omitting them still compiled and still
passed the tests that did not reach them.

Two consequences when writing the C and its test:

- Translate the call back into the struct assignment it came from (SHC then
  re-emits the same helper call); do not open-code three word copies.
- No DSL calling convention covers three operands (`Rori`/`Riro` cap at two), so
  `->with()` checks R4 and fails. Assert the operands inside `->do()` instead:
  the callback is bound to the simulator, so `$this->getRegister(1|2|0)->value`
  reads them directly.

## `->with()` covers stack arguments too -- declare all of them

`DefaultCallingConvention` overflows past R4-R7 into stack slots and
`ArgumentVerifier` handles `StackOffset`, so a 5-argument call like
`TaskSpawn_8c014ae8(tasks, action, &task, &state, alloc_size)` can have *every*
argument declared in `->with()`, `alloc_size` included -- a wrong one reports
`Unexpected argument ... in stack offset 0`. Don't hand-roll a check that reads
`@R15` in `do()`; the DSL already does it, with better messages.

`do()`'s `$params` is exactly the list passed to `->with()` (plain ints), so once
all five are declared, filling the out-params is just
`$this->memory->writeUInt32($params[2], U32::of($task))`. With no `->with()`,
`$params` is empty -- which is a reason to declare the arguments, not a reason to
go read registers.

The two out-param addresses are stack locals and *do* differ per object (for
`FUN_8c02845a`: `0xffffcc`/`0xffffc8` in the asm object, `0xffffd0`/`0xffffcc` in
the C one). That is what the project's `isAsmObject()` helper is for -- branch the
`->with()` values on it (see `tests/010fe8_heap/8c01102a_heapAlloc.php`), rather
than inventing a way to avoid knowing the addresses. Discover each one by running
with a placeholder and reading the `Expected ..., got 0x...` mismatch, and check
the failure's object file before concluding the two agree.

## `lint.sh` is not a per-function gate mid-unit

`scripts/lint.sh` starts with `make rebuild`, and per AGENTS.md the
non-matching full build is *expected* to fail from the moment a unit is
scaffolded until its last function is ported: other units import symbols the
half-written `.c` does not define yet, so the link dies on `UNDEFINED EXTERNAL
SYMBOL`. Running `lint.sh` after each function therefore reports a failure that
means nothing about the function just written.

Use `scripts/check_naming.py` as the per-function check instead -- it reads the
built objects, which are produced before the link fails. Save the full `lint.sh`
for when the unit is complete.

`scripts/check_private_decls.py` is also transiently wrong mid-unit: while some
functions are still asm, a `static` C function whose only in-unit caller has not
been ported yet is dropped entirely, so symbols look private that won't be once
the unit is finished.

## A unit can have more functions than its `.src` exports

`02c884_stop`'s `.src` exported 9 symbols, but the unit has 10 functions.
`drawStopMarker_8c02cd92` is never exported and never directly called -- its
address is only taken, by `RenderQueueDraw_8c0223ea` -- so Ghidra merged it
into the neighbouring function and it appears as a bare `LAB_` in the asm.

Count the unit's function labels, not its `.EXPORT` lines, and treat an
address-taken-only callee as a function in its own right. `028258` hit the same
thing from the other direction: decompiling turned up three functions
(`FUN_8c02833c`, `FUN_8c0283d4`, `FUN_8c0283e8`) missing from the stub list.
`02b464_grading` (2026-08-28/29) had six, all reachable only via
TaskSpawn pointers, including the master per-frame `taskCallback_8c02c072`.

**The inverse also happens:** an `.EXPORT`ed symbol that *isn't* a function at
all, just a label mid-body. `01fa78`'s `FUN_8c01fe84` (found 2026-08-29) is a
fallthrough point inside `FUN_8c01fbac` sharing its epilogue, exported and
Ghidra-shown as its own function, with zero BSR/JSR references anywhere in
the tree -- nothing actually calls it as a function; the "call" is a plain
fallthrough. Four units running now have had wrong Ghidra function boundaries
in one of these two directions. Neither the export list nor Ghidra's function
list is reliable; walk the asm for prologue/epilogue pairs directly:

    command grep -aoP '\.DATA\.L\s+\K(LAB|_?FUN)_\w+' <unit>.src | sort -u

A bare `LAB_` hit is a candidate function Ghidra folded into a neighbour.

## Test memory does not start zeroed

A test that never zero-fills the state it reads can pass for the wrong reason.
`var_segmentHasStop_8c2286a4` was typed `char[96]` and read byte-indexed in one function and
word-indexed (`SHLL2`) in another; the byte-indexed C was simply wrong, but its
test never discriminated, because whatever indexing it used read
garbage-nonzero out of uninitialised test memory.

Two habits fall out of this. Zero-fill the region a test depends on, and include
at least one case the code under test must *reject* -- a test where everything
passes proves nothing. Then confirm the test can actually fail: revert the C
body and watch it go red. When the asm object and the C object disagree, the asm
is the ground truth.

## Forcing a caller's own prototype can hallucinate an argument Ghidra can't verify

**Found in:** `026710_traffic` (2026-08-28)

`TrafficInitEntryState_8c026748` (`026710_traffic`) has no real float parameters
at all -- its whole first "instruction argument" shape was an artifact of my
own `set_function_prototype` calls on the function itself and on unprototyped
callees it invokes (`GroundQueryFindPolygon_8c020914`, `AttrQueryFindConvexPolygon_8c02e51c`). Each time I forced a
guessed signature, Ghidra's decompiler dutifully produced a plausible-looking
`in_frN`/`unaff_rN` value to satisfy it -- convincing pseudocode with zero
grounding, since neither function had a real prototype to check against. The
`--disasm` trace (obtained legitimately, per the skill's step-5 allowance,
while debugging the resulting failing tests) showed the truth: both callees
actually take a full `(x, y, z, out)` ground/junction-query quadruple built
from *entry fields* (`entry->0xf4`, a constant `0.0`, `entry->0xfc` /
`entry->0xf8`), and the caller's own function takes only `(entry, scriptIp)`
-- no floats at all.

**Fix:** setting a callee's prototype before decompiling a caller (per
`docs/ghidra-mcp.md`'s normal step 1) is fine for callees whose signature is
already established elsewhere. For a callee this project hasn't decompiled or
verified yet, treat Ghidra's resulting register names as an unverified guess,
not a fact -- write the test with your best guess, and when it fails with
`Unexpected argument ... in frN`, trust the simulator's reported value (it
reflects the real relocatable operand) over Ghidra's variable name, and
re-derive the call from the concrete `--disasm` trace instead of trying a
different forced prototype.

## An uninitialized-register write is real but not test-reproducible; use forceStop()

**Found in:** `026710_traffic` (2026-08-28)

`TrafficInitEntryState_8c026748`'s parked-vehicle path (`entry`'s script-header
word `== 10`) skips the vehicle path-walk entirely, leaving a path-segment
pointer local (`unaff_r11` / `seg`) and a distance local (`in_dr14` / `dist`)
unset -- yet the function unconditionally stores both to `entry+0x2b8` and
`entry+0x2c0/0x2bc` regardless of path taken. This is a genuine original-game
read of a register the function itself never initializes on that path. Float
registers (`FR0`-`FR15`) start at a fixed `0.0` in sh4objtest's simulator, but
general-purpose registers (`R0`-`R14`) are filled with `random_int()` at
startup (`Simulator.php`), so a `MOV.L Rn,@...` write of such a register (here
`R11`, holding the pointer) produces a *different* value on every single test
run -- there is no expected value to hardcode, not even a "same for both
objects" one from a single failing run.

**Fix:** don't chase this with a fixed expected value (it won't reproduce) or
skip the write silently (every write must be consumed by the expectation
queue, in order, or the next real expectation mismatches on address). Assert
everything deterministic up to the write immediately before the
uninitialized one, then call `$this->forceStop()` -- it stops the simulator
as soon as the queue empties, so the untestable write (and anything after it)
never executes. Note this in a comment so a future reader doesn't mistake the
short assertion list for incomplete coverage.

## Ghidra's SSA variable reuse can hide two different real values

**Found in:** `026710_traffic` (2026-08-28)

`TrafficUpdateHeading_8c026bc4` (`026710_traffic`) takes a float parameter Ghidra
showed as flowing straight into `njSqrt(param_1)` -- plausible, since the rest
of the function reads like a distance normalization. Probing the asm object
directly (mock `njSqrt`, read the `Unexpected argument ... Expected 25, got
500` mismatch) proved the parameter is completely unused: the real argument is
the inline `dx*dx + dy*dy` (`njHypot`'s expansion), computed from two struct
fields the function already reads for something else. A caller passing a
value nothing downstream reads is a real, if odd, property of the original
game -- not a decompilation mistake to "fix" by wiring the parameter in.

Separately, Ghidra's pseudocode reused one `fVar3` for two different real
values: the normalized heading component, then (after `fVar3 = halfWidth *
fVar3`) the corner-offset scaled by vehicle half-width. The final `acosf(fVar3)`
in the printed pseudocode reads as the *second* value, but probing (mock
`acosf`, check its argument) showed the real call receives the *original*,
pre-scale value -- the compiler kept it in a register Ghidra's SSA naming
collapsed into the same display name as the overwrite.

**Fix:** never trust that two appearances of the same Ghidra variable name
share the same real value once an intervening assignment reuses it, and never
assume an incoming parameter is actually read just because a call site's
argument shape "fits". Confirm both with a dual-object probe test: mock the
callee, assert a guessed argument, and read the real value off the failure's
"got" side rather than reasoning about it from the pseudocode alone.

## Proving a test can fail: gut the whole body, never inject an early `return`

A test is only worth its green if it goes red when the code is wrong. The check
is to replace the function's body and confirm the test fails on the C object
while the asm object still passes.

Replace the **entire** body by brace matching. Injecting an early `return` after
the declarations does not work: this is C89, and a function with initialized
declarations then produces a *compile* error. A compile error is not a
discriminating failure -- it looks like one in the output and will happily
manufacture false positives for tests that in fact assert nothing.

Record the literal failure line from the gutted run (`Pending expectations:
WriteExpectation`, `ReturnExpectation`, a count of pending Call/Write
expectations). "It failed" is not evidence; the pending-expectation line is.

Never register a test in `tests.php` for a function whose body is not live.

## A data-gap coincidence can point the base-symbol-plus-offset trap at the wrong base

**Found in:** `026710_traffic` (2026-08-28)

`TrafficMarkSignalIdsInUse_8c026dcc` reads a standalone Ghidra global `PTR_PTR_8c1bb88c`, and
`sectionB.src` then had an unexported 4-byte gap at exactly that address
(between the reservation for `var_8c1bb888` and the one for
`var_groundGridPrimary_8c1bb890`, both since folded away) -- a
plausible-looking match for the "invented base+offset global" pattern. It
was the wrong base: the dual-object test's `.src` object failed with
`Trying to read from unresolved relocation _var_currentCourse_8c1bb868`
once `var_8c1bb888` was wired up and resolvable, proving the real read
targets a completely different, already-known struct 0x24 bytes in --
`var_currentCourse_8c1bb868.macCpu1_0x24` (`CurrentCourse`,
`013ae8_route.h`), whose asset-file field doubles as a
per-scene-object-type table pointer once loaded and
`TrafficRelocatePlacementTable_8c026da4`-relocated.

**Fix:** an address falling inside a plausible-looking gap is only a
hypothesis. Confirm it the same way as any other guess here -- run the
dual-object test and read the concrete failure -- before writing the
supporting header/comment changes; an unresolved-relocation error naming a
completely different symbol than the one just wired up is a strong signal
the guessed base was wrong, not that the new global still needs seeding.

## A section-B symbol can only move between files at a B-range boundary

Moving `_var_8c227e1c`'s 4-byte reservation out of the middle of `sectionB.src`
into a unit's own B section shifted every later section-B symbol down 4 bytes.
The build still linked and every per-function test still passed -- only
`make -f Makefile.matching` caught it, with `Oops, build differs :/`.

Section B is one contiguous allocation whose order is fixed by the original
layout, so a symbol may only change owner if it sits at the start or end of the
range. Otherwise leave the reservation with its owner, `.IMPORT` it from the
unit's `.src`, take it from the owner's header in the C, and `setSize` it in tests
like any other section-B extern.

The move looks locally valid, which is what makes it dangerous. Run the
matching build after any data-ownership change, not just the tests.

## Renames must be swept across the whole graph, not just the unit

Per-function agents sweep only the files they touch. After a unit's functions
are renamed, other units' `.src`, `.c` and test files can still reference the
old `FUN_` names; the non-matching build tolerates this until the matching
build's link fails on them.

Two things make this hard to read. A stale object from before a file rename
(e.g. `build/output_matching/src/asm/<old>.obj`) keeps getting linked and
reports misleading undefined-symbol names -- `make -f Makefile.matching clean`
first. And the failure surfaces only at final link, long after the rename.

Sweep with `command grep -ran '<old_name>' src/ tests/` across the entire tree
when a unit's naming settles.

## A raw Ghidra data extraction isn't wireable just because it exists

**Found in:** `02af78_pre_data.src` (2026-08-28)

This file is a Ghidra extraction of the section D data spanning roughly
`8c044de0`..`8c04ab6c`, immediately preceding `02af78_event`'s own
`EventEntry` tables. Adding it to either Makefile's `SRCS` fails at link with
~590 `UNDEFINED EXTERNAL SYMBOL` errors, in **both** `Makefile` and
`Makefile.matching`. Most of what it `.IMPORT`s (section C consts, one
section B var) already has a real owner elsewhere in the tree, but that owner
doesn't yet make the symbol visible to this file:

- For an owner whose archived `.src` still holds the data (`01f3c0.src`,
  `01fa78.src`, `024280_bus_input.src`, `025870_demo.src`,
  `0289ac_objects.src`), the label exists but is never `.EXPORT`ed -- nothing
  outside that file has needed it yet.
- For the C-based `Makefile`, an owner that *is* decompiled (e.g.
  `01bb48_vm_game`) only ports data its decompiled C functions actually
  reference; a lot of this span's consts aren't referenced by any decompiled
  function yet, so they don't exist in the `.c` at all even though the
  archived `.src` still has them.
- One import, `_var_8c2260ac`, isn't even current: the live name is
  `_var_lcdAnimBus_8c2260ac` (`01bb48_vm_game`). This file is a
  point-in-time snapshot from before that rename, not a maintained source.

So this isn't a link-order problem (contrast the separate, already-resolved
`02af78`/`03bd80_sectionD` `EventEntry`-ownership gap) -- it needs real work
in each true owner (`.EXPORT` + C definition) before this file's imports
resolve, split out block-by-block with `scripts/move_data.py` per
`.claude/skills/move-data/SKILL.md`. Until that happens the file stays
unwired, with a comment at its head pointing here.

## `02f320_replay_codec`: EXTS.W blocks testing most of this unit against the asm object

**Found in:** `02f320_replay_codec` (2026-08-28)

This unit is a bit/byte-stream codec (LZ-style compressor with an adaptive
code table, likely lzhuf-family) working on `Sint16` counters. SHC's asm
output habitually re-sign-extends a 16-bit value with `EXTS.W` right after
loading/storing it via `MOV.W` (which already sign-extends into the 32-bit
register on real SH4) -- e.g. `FUN_8c02f3a0` (`GetBit`-shaped): decrement a
`Sint16` counter, store it back, then `EXTS.W`+`CMP/PZ` to branch on its
sign. `sh4objtest` does not implement `EXTS.W` (documented limitation), so
any test that calls a function whose asm executes one crashes with `Unknown
instruction 633f` -- and this happens on the **archived original** asm object,
which cannot be changed, not on anything the C side does. There is no way to
route around it by picking different test inputs; the instruction executes
unconditionally on every call.

Confirmed only `ReplayCodecInit_8c02f320` (no `Sint16` branches) is testable
so far. Every other exported function in this unit reads a `Sint16` counter
and branches on it in the same idiom, so this likely blocks dual-object
testing for most of the unit until `sh4objtest` gains `EXTS.W` support.

**Resolved:** `sh4objtest` v0.1.45 added `EXTS.W`. The rest of the unit
decompiled normally against the asm object afterward -- see the entry below for
the one other instruction-coverage gap hit along the way.

## `02f320_replay_codec`: confirms the lzhuf/LZW hypothesis; `sh4objtest` gap on `MOV.W @Rm+,Rn`

**Found in:** `02f320_replay_codec` (2026-08-29)

Finishing this unit's remaining 15 functions confirmed the codec is
**LZW** (not lzhuf): `extendDict_8c02f740`/`ReplayCodecPack_8c02f934`/
`ReplayCodecUnpack_8c02fa14` build a growing (parent code, appended byte)
dictionary via hash-chained buckets (`lzwFindChild_8c02f636` /
`lzwInsertChild_8c02f668` / `lzwRemoveChild_8c02f6ac`), decode a code by
walking `var_dictParent_8c229bae` (parent-of) back to a literal byte, and once the
fixed 4096-entry table fills, evict the least-recently-used code (tracked by
a `listInsert_8c02f58a`/`swapNodes_8c02f556` recency list, move-to-front on
reuse) -- classic LZW with LRU replacement, not an adaptive-Huffman tree.
`readCode_8c02f892`/`writeCode_8c02f824` are the classic escalating-code-width
primitives (9-bit codes growing as the table fills, an extra control bit
picking literal-byte vs. dictionary-code).

Separately: a straightforward C copy loop (`for (i...) dest[i] =
srcWordArray[i];`, reading a `Sint16` scratch buffer and truncating to a
byte) made SHC emit `MOV.W @Rm+,Rn` (word load, post-increment) --
`sh4objtest` doesn't implement that addressing form and throws `Unknown
instruction NNNN` (varies with the register-number bits in the opcode).
Confirmed by dumping the compiled listing with `shc -code=asm` (`Makefile.test`
already does this into `build/output_test/src/<unit>.obj.src`) and reading the disassembly around the
crash address printed by `-v --disasm`. The *original* archived asm never
uses this addressing mode anywhere in the unit -- circumstantial evidence
it's cold/rare in real SHC-era output, not just untested in `sh4objtest`.
Two different rewrites of the loop (plain indexed, split start-index) still
produced the same word-post-increment codegen; only reading through an
explicit `Uint8*` alias (byte loads only, already well-exercised elsewhere)
avoided it. Prefer a byte-typed view for tight array-copy loops over a
16-bit scratch buffer when only the low byte is ever meaningful.

## Ghidra reorders writes; statement order in its output is not evidence

In `hitWall_8c023bea` (023938_bus_drive) Ghidra emitted the
`var_wallHitBits_8c228660 |=` before the `speed_0x27c` write in each branch; the asm does
`sdMidiPlay` -> speed write -> `var_wallHitBits_8c228660` write. Nothing about the output
looks suspicious, so this only surfaced as an unexpected-write-order failure
against the asm object. Whenever two writes are both observable, check their order
against the asm rather than reading it off the decompile, and assert the order
in the test.

## `FCMP/GT Fm,Fn` tests `Fn > Fm`, not `Fm > Fn`

The operand order reads backwards from the mnemonic. Getting it inverted flips
a comparison's sense while still compiling and still looking right -- in
`TrafficDriveParkedTask_8c02656a` (025b98_traffic_drive) it turned a "free the
task when the entity is FAR from the reference point" into "when it is near".
Only the dual-object test caught it. Check the operand order against the SH4
manual whenever a float comparison's branch sense matters.

## Float literals need full round-trip precision to byte-match

Porting section C/D float data, a "clean-looking" constant may be one ULP off
the value you would write by hand: `02e2dc`'s box tables needed
`3.0500001907348633f`, not `3.05f`. Write the exact round-tripped decimal from
the original bytes and let `dcdiff.py` confirm, rather than tidying the literal.

## A `static const float` can break the data match

Introducing one for a literal (e.g. a fade rate) puts it in a constant section
the original object does not have, so `check_data_match.sh` reports a section
the asm never emitted. Inline the literal at its use site instead. Hit in
`02d19c`.

## Mocking `__divls` with `UInt::div()` gives unsigned division

The helper divides unsigned, so a mock built on it is silently wrong for
negative dividends -- it passes every non-negative case and fails only at a
clamp boundary. Do the signed division manually (`intdiv` on `signedValue()`).
Hit in `021b9c_tile_draw`'s clamp-boundary test.

## `field_0xNN` names are not unique across structs; blind sed clobbers them

Offset-suffixed member names collide. `field_0x2dc` exists on both `BusState`
and `TrafficEntry` and means different things; `MenuState.field_0x1c` and
`InstructorDialogTask.field_0x1c` sit in the same file; every task-state struct
overlays the generic `Task`'s `field_0x08`/`0x0c`/`0x10`/`0x14`/`0x1c`.

A repo-wide `sed s/field_0xNN/newName/` therefore renames unrelated fields on
other types, and it compiles cleanly -- the offsets are valid on both structs.
Three separate rename agents hit this; two caught it only on a full-file
cross-check afterwards, and the `BusState`/`TrafficEntry` case additionally
disproved the assumption that the collisions stopped at the end of the shared
prefix (`rpmRampAngle_0x2e4` collided past it).

Scope every substitution by the accessing variable's type, then re-read the
whole file. The tests do not reliably catch it: a wrongly renamed field of the
same type and offset still passes.

The same collision misleads *reasoning*, not just sed. `CourseConfig.ukn_0x24`
and `CurrentCourse.macCpu1_0x24` share the offset spelling, so a naming pass
tried to name the first from what the second is used for -- the three per-route
tables it holds would have been called traffic preset tables. They are
`LineBusNode` tables: `requestCourseAssets_8c014088` assigns
`course->lineNodes_0x0c = config->lineNodes_0x24`, moving the value to a
*different* offset, and `macCpu1_0x24` is filled a line later from an entirely
separate file field. Follow the assignment, never the matching suffix; the sizes
confirmed it independently, all three being exact multiples of `sizeof(LineBusNode)`.

## A hand-written string constant is usually a literal pool; inline it

`025870_demo`'s 28 place names started life as hand-written `Uint8` byte
arrays, but they are really the compiler's own literal pool: address order is
first-use order, identical strings are deduplicated to one copy (104 of its
131 shots share the empty caption), and each is padded to a 4-byte boundary.
That is what the original source looked like -- the names were written inline
in the tables -- and written inline again the pool comes back byte-identical.

The tell that a run of constants is a pool: every size is exactly
text + terminator rounded up to 4, the empty one appears once no matter how
many places use it, and address order tracks the order the tables reference
them.

Two things that do *not* work, if the rebuilt pool ends up short at the tail:

- adding an explicitly declared array to make up the difference -- SHC emits
  declared arrays *before* the pool, so it shifts everything
  (`[C] data differs at 0x0`);
- padding the last literal in the strings header (`"...\0\0\0"`). It does
  produce a match, but it puts a layout artifact inside translatable text, and
  it is fixing the wrong end -- see below.

## `.RES.B` after a string's terminator is section fill, not data

The archived asm for the last constant in a section ends with the alignment
fill the assembler inserted before the next `.SECTION`:

    ;.DATA.B     H'00
    ;.RES.B      3
    .DATA.B  ... H'4F, H'00, H'00, H'00, H'00

The disassembler's own commented-out original separates the two -- terminator,
then `.RES.B 3` -- but the emitted line folds the fill in as data. That makes
the archived object 3 bytes longer than any C that reproduces the real
content, and the diff looks like a missing pad in the C.

It is the archive that is wrong. Dropping the fill from the `.src` leaves
`[C] match (357 bytes)` against the C object *and* keeps the matching build
byte-identical, because the linker re-inserts the same fill at the
`.SECTION D, DATA, ALIGN=4` boundary. Check for this before contorting the C
to reproduce a tail that was never data.

## SHC emits a second, pooled copy of any literal named inside `sizeof`

Every archived Shift-JIS constant obeys one rule -- text + terminator, rounded
up to 4 -- so `strings.h`'s hand-written `TEXT_SJIS_SIZE(n)` looks derivable:

```c
#define TEXT_SJIS(sym, text) const char sym[(sizeof(text) + 3) & ~3] = text
```

It isn't. Naming the literal in `sizeof` makes SHC materialise it into the
string pool *as well as* into the array, so section C comes out holding both:
`025870_demo` went 360 -> 717 bytes and `01e27c_practice_menu` 72 -> 142, both
losing a data match they had. One mention (the initializer alone) pools
nothing; two mentions pool one copy.

There is no way to derive the length without naming the literal twice, so
where a sized array is genuinely needed the count stays written down. The
better escape is usually not to need one: inline the literals and let the
compiler lay the pool out, as `025870_demo` now does.

## `const` on the array, not just the pointee, moves a pointer table out of D

`STATIC const char *const tbl[]` lands in section C; `STATIC const char *tbl[]`
lands in D. The archive puts `01f3c0_ending`'s two credit-roll tables in D, so
the outer `const` -- added for tidiness -- moved 152 bytes into the wrong
section and took the strings they point at with them.

## An `= {0}` on a local aggregate costs a zeroed section C template

SHC implements the initializer by memcpy-ing a zero-filled constant onto the
stack, so `Uint8 buf[768] = {0}` puts 768 zero bytes in section C.
`015034_text`'s `unpackGlyph_8c015110` had two, 2816 bytes the original object
does not have -- it zeroes with a runtime loop instead. Clear what actually
needs clearing inside the loop that already walks the array.

## An empty `.SECTION D` in the archive is a disassembler artifact

`0129cc_pause.src` declared `.SECTION D,DATA,ALIGN=4` with nothing under it,
giving the archived object a zero-length D section that SHC never emits for a
TU with no initialized data. Delete the directive; the matching build is
unchanged, since a zero-length section contributes no bytes.

## Our own section B cannot tell you where an object begins and ends

The archives were disassembled from a linked binary, where a literal pool word
is just an address; the disassembler names it against whatever symbol table we
supply. Because `sectionB.src` had a label on every 4 bytes of
`0x2285c4..0x228660`, a folded single-field access -- `base + H'c` -- was
rendered as `.IMPORT _var_8c2285d0`, and 39 fields of one struct looked like 39
independent globals with their own `.RES.B` runs. Commit `91d6aa3` acted on that
reading and split the object across 47 files.

The evidence that settles it is displacement, not labels. Collect every constant
offset each TU reaches off a pool entry holding the candidate base: the object
spans the union, and ends at the first address a TU loads as its own base. Here
the union was gapless from `+0x00` to `+0x98`, and `wallHitBits_8c228660` is
loaded as a base by three TUs -- so the object is exactly 156 bytes. A gapless
union over dozens of words cannot come from independent globals, since SHC has
no way to know two unrelated externs are adjacent.

Three encodings carry those offsets, and missing one shrinks the object:
`@(H'd,Rn)` reaches only `+0..+60`; `MOV #H'xx,R0` with `@(R0,Rn)` holds a
*constant* in R0, not a variable index; and past `+127` the constant comes from
a `.DATA.W` pool entry via `MOV.W <pool>,R0`.
