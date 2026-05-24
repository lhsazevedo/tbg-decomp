#!/usr/bin/env python3
"""Remove unused .IMPORT directives from SH4 assembly .src files."""

import re
import sys
from pathlib import Path

IMPORT_RE = re.compile(r'^\s+\.IMPORT\s+(\w+)')


def process_file(path: Path, dry_run: bool = False) -> int:
    text = path.read_bytes().decode('shift_jis')
    lines = text.splitlines(keepends=True)

    imports = {}  # symbol -> line index
    for i, line in enumerate(lines):
        m = IMPORT_RE.match(line)
        if m:
            imports[m.group(1)] = i

    removed = 0
    keep = []
    for i, line in enumerate(lines):
        m = IMPORT_RE.match(line)
        if m:
            symbol = m.group(1)
            # Check if symbol appears anywhere other than this import line
            pattern = re.compile(r'\b' + re.escape(symbol) + r'\b')
            used = any(
                pattern.search(other_line)
                for j, other_line in enumerate(lines)
                if j != i
            )
            if not used:
                print(f'  removing: {line.rstrip()}')
                removed += 1
                continue
        keep.append(line)

    if removed and not dry_run:
        path.write_bytes(''.join(keep).encode('shift_jis'))

    return removed


def main():
    args = sys.argv[1:]
    dry_run = '--dry-run' in args
    paths = [a for a in args if not a.startswith('--')]

    if not paths:
        print('Usage: clear_unused_imports.py [--dry-run] <file.src> ...')
        sys.exit(1)

    total = 0
    for p in paths:
        path = Path(p)
        print(f'{path}:')
        n = process_file(path, dry_run=dry_run)
        if n == 0:
            print('  no unused imports')
        total += n

    print(f'\n{"Would remove" if dry_run else "Removed"} {total} unused import(s).')


if __name__ == '__main__':
    main()
