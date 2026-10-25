---
name: unit-pass
description: Review one decompiled unit's names against what the code actually does, then cut its comments down to the facts the names and code don't already say. Use when asked to review, rename, deslop or trim the comments of a unit. Code shape is out of scope (code-shape-pass).
---

# Unit pass

AI decompilation leaves two kinds of debris. Names get picked early, from the first
caller that was read, and survive after the function turned out to do something else.
Comments get written as note-taking: who calls this, which field it touches, how it was
figured out. Once the names are right, most of those comments say nothing the reader
doesn't already have.

So the pass runs in that order, names first. Trimming comments assumes the names carry
the meaning; do it the other way round and you delete the only place a misnamed thing
was explained.

The pass never changes what code does. Code shape belongs to `code-shape-pass`.

## Before you start

1. **Get a green baseline.** The matching build, `./build.py`, `./scripts/lint.sh`, and the
   unit's tests (`./scripts/run_tests.sh -p /app/tests/<unit>`). A pass that starts red
   can't tell its own breakage from the pre-existing kind.
2. **Have the asm open.** `src/asm/decompiled/<unit>.src` is the authority on what a
   function reads and writes. A name or comment that disagrees with the asm is wrong,
   however confident it sounds.
3. **Know the domain.** `docs/gameplay.md` has the game terms (run, course, route,
   stop, grading). Prefer the game's own words to invented ones.

## The worklist

One row per symbol the unit owns: functions, globals, types and their fields,
macros. Keep it in a temp file in your scratchpad:

    symbol | name verdict | comment verdict | note

Name verdicts: `ok`, `renamed -> <new>`, `unsure` (a better name exists but the code
doesn't prove it -- leave it and say why). Comment verdicts: `kept`, `trimmed`,
`removed`, `none`. The pass is done when every row has both verdicts.

## Step 1: names

For each symbol, read what it does or stores, not how its callers use it. Then ask
whether the name says that.

- **Name the behaviour, not the first caller.** `RenderStartFadeIn` is right;
  `RenderCalledByTitle` never is. A global is named by what it holds, not by who
  writes it.
- **Don't repeat the unit prefix.** `BusCameraUpdate`, not `BusCameraUpdateCamera`.
- **Use the game's terms** where they fit, and the SDK's where the code wraps the SDK.
- **Field names keep their `_0x<off>` suffix.** Only the stem changes.
- **When unsure, don't rename.** A vague name is better than a wrong one. `FUN_` /
  `var_8c...` are fine to leave when the code doesn't say enough; mark the row `unsure`.
- **Unit name:** rename the unit only if its name is misleading for the whole of it,
  following `git show 34ea3e8` (files, guard, `@unit`, prefix, `build.py` UNITS, tests.php).

Rename by script, never by hand: a map of old -> new over src/, tests/, tests.php,
docs/ and .claude/skills/. Asm names carry a leading underscore, so match
`(\b|_)Old_8c`, never a bare `\b` before a name. Rename a test file named after a
renamed function (`8c<addr>_<Name>.php`) and its `call()` entrypoint too.

Commit the renames before touching comments.

## Step 2: comments

Keep a comment only if it states something the name and the code don't:

- units and scales (`/* frames */`, `/* 1/4096 turns */`), ranges, sentinel values
- magic numbers and what they select
- file and wire formats, layouts the type can't express
- original-game quirks and bugs kept on purpose, and why odd-looking code is that way
- a cross-unit protocol that isn't visible in any signature

Cut:

- caller lists ("Called by X and Y", "used by 02412c")
- restating the implementation ("loops over the table and sets flags")
- which fields a function reads or writes, when the code shows it
- how something was worked out, Ghidra names, old addresses, former owners, and
  history ("was in sectionB", "renamed from")
- a docblock on a function whose name already says it all: delete the whole block

Keep what survives short, and stop once the fact is stated -- no causal tail the reader
can infer. Header comments on public declarations follow the same rule: one line, and
only if the name isn't enough.

Before/after, from `023938_bus_drive.h`:

    /* Computes 10 corner/lookahead ground-sample points around the bus and
     * queries each through the ground-query callback in groundProbeFn_0x2c8, filling
     * groundSamples_0x190; also reseeds posHistory_0x100[0]/[1] and the heading
     * unit vector headingDirX_0x274/0x278. Called by placeBus_8c023310/
     * BusInitStart_8c023610 (023310_bus_init) and BusTask_8c022bdc (022bdc). */
    void BusDriveSampleGround_8c023938(void);
    ->
    /* Also reseeds the position history and heading. */
    void BusDriveSampleGround_8c023938(void);

The name covers the sampling; the reseeding is the one thing it doesn't say.

## Verification

After the renames, and again after the comments:

- `./build.py matching` -> `Yay! Build matches :)`
- `./build.py` -> `Project built :)`
- `./scripts/lint.sh` -> all blocking checks passed, and no new `check_test_naming`
  mismatches
- `./scripts/run_tests.sh --parallel` -> everything passes

Comments can't break the build, but a stray `*/` or non-ASCII character can: files are
Shift-JIS, so keep edits ASCII and check with `command grep -a`.

## Commits

Two per unit, title only:

- `Review <unit> names`
- `Trim <unit> comments`

## Stop conditions

- **A rename would need a behaviour change** to make sense (e.g. the function really
  does two things). Leave the name, note it; that's a code-shape or design question.
- **The asm contradicts a comment** you were about to keep. Fix the comment to match
  the asm, and report it; the C may be wrong too.
- **A comment documents a known original-game bug.** Keep it, even if it looks like
  note-taking.

## Report

Verdict counts, the rename list (old -> new), anything `unsure`, and every place the
asm disagreed with a comment or name.
