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

# Test (full suite)
./scripts/run_tests.sh

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
  asm/<addr>.src             # Untouched assembly (not yet decompiled)
  asm/decompiled/<addr>.src  # Parallel asm stub used only during unit tests
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
| Functions | `_FUN_8c<addr>` | `_FUN_8c012718` |
| Variables | `_var_name_8c<addr>` | `_var_resetRequested_8c157a78` |
| Tests | `8c<addr>_name.php` | `8c012718_FUN.php` |

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

## Adding a New Unit

1. Create `src/<addr>_name.c` and `src/asm/decompiled/<addr>_name.src`
2. Add assemble/compile steps to `scripts/run_tests.sh`
3. Add test group (tests + objects) to `tests.php`
4. Add C source to `SRCS` in `Makefile`

## Detailed Guides

- **Writing tests** → `tests/AGENTS.md`
- **Moving data between files** → `.agents/skills/move-data/SKILL.md`
- **Adding serial debug logging** → `.agents/skills/add-serial-logging/SKILL.md`
- **Environment setup** → `docs/setup.md`
