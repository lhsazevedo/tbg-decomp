# Lessons Learned

Indexed, non-obvious findings from decompiling this codebase. Each entry is a
mistake made once, or a quirk of the toolchain/original build, worth not
re-discovering. Add an entry whenever you hit something surprising; keep each
one short and dated with the unit where it was found.

## Index

- [Struct fields can be separately-imported symbols in asm](#struct-fields-can-be-separately-imported-symbols-in-asm)
- [Calls to sibling functions in the same TU are still mocked](#calls-to-sibling-functions-in-the-same-tu-are-still-mocked)
- [A nested single-statement `if(cond){break;}` can compile to unreachable bytes](#a-nested-single-statement-ifcondbreak-can-compile-to-unreachable-bytes)

## Struct fields can be separately-imported symbols in asm

**Found in:** `02af78` (2026-07-12)

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

**Found in:** `02af78` (2026-07-12)

A function that calls another function decompiled earlier *in the same
`.c`/`.src` file* (e.g. `scanUnlockCandidates_8c02b03c` calling
`hasProgressFlag_8c02afbe`, both in `02af78.c`) does **not** execute the
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

**Found in:** `02af78` (2026-07-12)

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

**Fix:** rewrite the first arm to match the second arm's shape -- combine
the two conditions with `&&` into a single `if (mode_matches && some_call(...)
!= 0) { break; }` instead of nesting. Same behavior, but the compiler now
generates one direct conditional branch with no leftover dead bytes, and the
line becomes reachable/coverable. Worth trying this rewrite whenever a
`break;` inside a nested `if` stays stubbornly uncovered despite a test that
should hit it.
