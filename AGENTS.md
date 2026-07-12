# Tokyo Bus Guide Decompilation

Dreamcast (SH4) game decompilation. Goal: functional equivalence, not byte-for-byte matching.

## Environment

All build/test commands run inside Docker:
- `./docker-shell.sh` — interactive shell (working dir `/app`)
- `./docker-run.sh <cmd>` — run a single command non-interactively

## Key Commands

```bash
# Build
make

# Test (full suite; --parallel speeds this up, sh4objtest v0.1.28+)
./scripts/run_tests.sh --parallel

# Test (single file)
./scripts/run_tests.sh -c /app/tests/path/to/test.php

# Test (single file + debug trace)
./scripts/run_tests.sh -c /app/tests/path/to/test.php -d
```

A matching build prints `Matching project built! \o/`; a non-matching build prints `Project built :)`.

## Directory Map

```
src/
  <addr>_name.c              # Decompiled C unit
  <addr>_name.h              # Public interface for that unit
  asm/<addr>.src             # Original assembly (not yet decompiled)
  asm/decompiled/<addr>.src  # Original assembly, decompiled (used for archival and unit tests)
  includes.h                 # Shared macros (STATIC, logging levels)
tests/
  <unit>/<addr>_name.php     # One PHP test file per function
  AGENTS.md                  # ← DSL reference and test-writing guide
tests.php                    # Test suite registry — add new groups here
scripts/run_tests.sh         # Compile + run all tests
Makefile                     # Full build; SRCS list drives compilation order
.agents/skills/move-data/    # Skill: migrate data ownership between files
.agents/skills/add-serial-logging/  # Skill: add LOG_* serial debug logging to a unit
docs/setup.md                # One-time environment setup
```

## Naming Conventions

| Thing | Pattern | Example |
|---|---|---|
| Unit files | `<hex_addr>_name.c` | `012f44_reset.c` |
| Functions | `Name_8c<addr>` (single camel/PascalCase word, no `_` in the stem) | `clearTasks_8c014a9c` |
| Not-yet-named functions | `FUN_8c<addr>` | `FUN_8c012718` |
| TaskAction callbacks | `<Desc>Task_8c<addr>` | `TitleTask_8c015ab8` |
| Variables | `var_name_8c<addr>` | `var_resetRequested_8c157a78` |
| Constants | `const_name_8c<addr>` | `const_peripheral_8c033318` |
| Data initializers | `init_name_8c<addr>` | `init_courseTable_8c043ca4` |
| Not-yet-named data | `var_8c<addr>` / `const_8c<addr>` / `init_8c<addr>` | `var_8c157ad4` |
| Tests | `8c<addr>_name.php` | `8c012718_FUN.php` |

The `8c<addr>` suffix is always the full address, never shortened. A function or
variable is checked against these patterns by `scripts/check_naming.py`, which
inspects the built objects' exported symbols directly (see that script's docstring)
rather than grepping source. Two kinds of names are exempt from the address suffix:

- **SDK-mirrored functions**: a small, explicit allowlist in `check_naming.py` for
  functions whose name must match an official SDK header verbatim (e.g. `BupInit`,
  `main`, `scif_flush`) — the compiler/linker already enforces these are correct, so
  the address suffix would be redundant.
- Everything else must have the address suffix; there is no other exemption.

`scripts/check_test_naming.py` checks (informationally -- it never fails the build)
that a per-function test file's `$this->call(...)` entrypoint matches its own
filename, to catch a test left behind under its old name after the function it
covers got renamed.

Every non-`STATIC` ("public") function must be declared in that unit's
`<addr>_name.h` — create the header if the unit doesn't have one yet. `STATIC`
functions (private to the unit) do not need a header declaration.

## C Source File Structure

Sections in order, each with a banner like the one below; omit empty sections:

```c
/* ====================
 * Section Title
 * ====================
 */
```


1. Compiler Definitions
2. Type Declarations
3. Non-initialized Globals
4. Initialized Globals
5. Forward Declarations
6. Functions

## Key Conventions

- `STATIC` macro (from `includes.h`): expands to `static` in production, empty in unit tests so test harness can access the symbol.
- Private globals in asm: export only under `.AIFDEF UNIT_TESTING` / `.AENDI`.
- Each unit has two object files: `<addr>_src.obj` (asm) and `<addr>_c.obj` (C).
- **ASCII only** in `src/` and `tests/` files — they are Shift-JIS encoded; non-ASCII characters (including Unicode arrows `→`, smart quotes, etc.) will corrupt the file. Use plain ASCII alternatives (e.g. `->` instead of `→`).
- **No local `extern` declarations in `.c` files.** Every external symbol must come
  from an `#include`d header: the owning unit's header if it already has one, or a
  new minimal header for a still-undecompiled `asm` unit if it doesn't (declare
  only the symbols actually referenced, not every export of that object). C never
  checks an `extern` declaration against the real definition across translation
  units, so a stale one silently compiles and links as long as the name matches
  (this has caused real bugs, e.g. a header declaring `BupInit` when the actual
  function was `bupInit_8c014b8c`). Checked by `scripts/check_naming.py`, which
  also flags an `extern` for a symbol nothing in that file actually references
  (dead code) and an `extern` for a function/variable defined later in the same
  file (that's a forward declaration, not a cross-unit extern -- write it as a
  plain declaration with no `extern` keyword, in the file's own Forward
  Declarations section).

## Adding a New Unit

Initial setup, before decompiling any function (`git mv` the asm, not `mv` --
the tests dir can be plain `mkdir` since it starts empty):

1. `git mv src/asm/<addr>.src src/asm/decompiled/<addr>.src` (rename to
   `<addr>_name.src` once a name is chosen; ok to leave hex-only if not yet).
2. Create `src/<addr>.c` — can start as just `#include <shinobi.h>`, no
   functions yet.
3. `mkdir tests/<addr>` (empty for now; first test file gets added, along
   with the test group in `tests.php`, once you decompile the first
   function).
4. In `scripts/run_tests.sh`, add an `assemble`/`compile` stanza pointing at
   the two files from steps 1-2.
5. In `Makefile` (non-matching), swap the `SRCS` entry from
   `src/asm/<addr>.src` to `src/<addr>.c`.
6. In `Makefile.matching`, swap the `SRCS` entry from `src/asm/<addr>.src` to
   `src/asm/decompiled/<addr>.src` (same content, just the new path —
   matching build keeps using the archived original asm until the unit's C
   is proven byte-matching).

Steps 5-6 intentionally break `make` (full non-matching build) until every
function in the unit is decompiled — other units still import the
not-yet-defined symbols. That's expected; use `./scripts/run_tests.sh` to
verify in the meantime instead of a full `make`.

Then, per function: decompile in `src/<addr>_name.c`, add its test file
under `tests/<addr>/`, and register it in the matching group in `tests.php`
(create the group on the first function).

## Detailed Guides

- **Reading/maintaining Ghidra via MCP** → `docs/ghidra-mcp.md`
- **Gameplay reference (what the code implements)** → `docs/gameplay.md`
- **Next decompilation targets (relocation-graph analysis)** → `docs/next_units.md`
- **Lessons learned (non-obvious toolchain/asm quirks)** → `docs/lessons_learned.md`
- **Writing tests** → `tests/AGENTS.md`
- **Moving data between files** → `.agents/skills/move-data/SKILL.md`
- **Adding serial debug logging** → `.agents/skills/add-serial-logging/SKILL.md`
- **Environment setup** → `docs/setup.md`
