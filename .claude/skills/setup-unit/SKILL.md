---
name: setup-unit
description: Scaffold a new translation unit before decompiling any of its functions -- move the raw asm under decompiled/, create the empty C file and test dir, and wire it into both Makefiles, run_tests.sh, and tests.php. Use when starting on a fresh src/asm/<addr>.src, before the decompile-function skill.
---

# Set Up a New Unit

One-time scaffolding for a `src/asm/<addr>.src` you are about to start decompiling.
This precedes the [decompile-function](../decompile-function/SKILL.md) skill: it makes
the unit build from C (non-matching) and from the archived asm (matching), and gives
tests somewhere to live. No functions are ported here.

A name may not be chosen yet -- keep everything hex-only (`018644.src`, `018644.c`,
`tests/018644/`). Rename to `<addr>_name.*` later, once a ShortUnit is decided.

## Steps

Given `<addr>` (e.g. `018644`):

1. **Move the asm** into the decompiled archive (use `git mv`, the matching build
   reads it from here):
   ```bash
   git mv src/asm/<addr>.src src/asm/decompiled/<addr>.src
   ```

2. **Create `src/<addr>.c`** -- just the include, no functions yet:
   ```c
   #include <shinobi.h>
   ```

3. **Create the test dir** (plain `mkdir`, starts empty):
   ```bash
   mkdir tests/<addr>
   ```

4. **run_tests.sh** -- add an assemble/compile stanza next to the others (before the
   final `$sh4objtest suite` line):
   ```sh
   # <addr>
   assemble  src/asm/decompiled/<addr>.src
   compile  src/<addr>.c
   ```

5. **Makefile** (non-matching) -- swap the `SRCS` entry from asm to C:
   ```
   src/asm/<addr>.src \   ->   src/<addr>.c \
   ```

6. **Makefile.matching** -- swap the `SRCS` entry to the archived asm path:
   ```
   src/asm/<addr>.src \   ->   src/asm/decompiled/<addr>.src \
   ```

7. **tests.php** -- add a group at the end of `groups`. Objects are named
   `<addr>_src.obj` / `<addr>_c.obj` (basename of the asm/C files). The `tests` array
   starts empty; the first `.php` file lands here once you decompile a function:
   ```php
   [
       "tests" => [
       ],
       "objects" => [
           "build/output_test/<addr>_src.obj",
           "build/output_test/<addr>_c.obj",
       ]
   ],
   ```

## Verify

```bash
./docker-run.sh ./scripts/run_tests.sh
```

The new object should assemble and compile with 0 errors/warnings and the suite still
pass -- an empty test group builds its objects but asserts nothing.

Do **not** run a full `make` (non-matching): steps 5-6 intentionally break it until
every function in the unit is decompiled, because other units still import symbols the
empty C file doesn't yet define. Use the test suite to verify in the meantime.
