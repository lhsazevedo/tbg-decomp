#!/usr/bin/env python3
"""Check that a test file's entrypoint call() matches its own filename.

Test files are named `<addr>_<name>.php` or `8c<addr>_<name>.php` (see AGENTS.md
"Naming Conventions"). This checks that at least one `$this->call('_Symbol_8c...')`
in the file targets an address matching the filename's <addr>, and that the
symbol's own name stem loosely matches the filename's <name> -- catching stale
filenames left behind by a rename (the entrypoint was renamed, the test file
wasn't).

Group-level test files that exercise a whole translation unit rather than one
function (e.g. `12324_task.php`, `015ab8_title.php`) are out of scope -- they're
identified by not matching the `<addr>_<name>.php` shape at all and are skipped.

Report-only (always exits 0): most mismatches today are `..._FUN.php` files left
over from before their function got a real name, and renaming the ~130 test files
to a single address-suffix style is a separate, not-yet-scheduled task (see
AGENTS.md). This just surfaces drift for a human to act on.

Usage: scripts/check_test_naming.py [tests_dir]
"""
import re
import sys
from pathlib import Path

FILENAME_RE = re.compile(r"^(?:8c)?([0-9a-fA-F]+)_([A-Za-z][A-Za-z0-9]*)\.php$")
CALL_RE = re.compile(r"->call\(\s*['\"]_?([A-Za-z][A-Za-z0-9]*)_(?:8c)?([0-9a-fA-F]{4,8})['\"]")


def normalize_addr(addr):
    return addr.lower().lstrip("0") or "0"


def check_file(path):
    m = FILENAME_RE.match(path.name)
    if not m:
        return None  # not a per-function test file, skip
    file_addr, file_name = m.group(1), m.group(2)
    file_addr_norm = normalize_addr(file_addr)

    text = path.read_text(encoding="shift_jis")
    calls = CALL_RE.findall(text)
    if not calls:
        return None  # no call() found (shared helper file, etc.), skip

    same_addr_calls = [(name, addr) for name, addr in calls
                        if normalize_addr(addr) == file_addr_norm]
    if not same_addr_calls:
        called = ", ".join(f"{n}_{a}" for n, a in calls)
        return f"no call() targets address {file_addr} from filename (calls: {called})"

    file_name_folded = file_name.lower()
    for name, addr in same_addr_calls:
        if name.lower() == file_name_folded or file_name_folded in name.lower() or name.lower() in file_name_folded:
            return None

    first_name = same_addr_calls[0][0]
    return f"filename says '{file_name}' but call() targets '{first_name}_{same_addr_calls[0][1]}'"


def main():
    tests_dir = Path(sys.argv[1] if len(sys.argv) > 1 else "tests")
    violations = []
    for path in sorted(tests_dir.rglob("*.php")):
        msg = check_file(path)
        if msg:
            violations.append(f"[{path}] {msg}")

    if violations:
        print(f"{len(violations)} test filename/entrypoint mismatch(es) (informational):\n")
        for v in violations:
            print(f"  {v}")
    else:
        print("OK: test filenames match their call() entrypoints")


if __name__ == "__main__":
    main()
