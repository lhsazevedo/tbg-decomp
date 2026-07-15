#!/usr/bin/env python3
"""Move a span of asm data blocks from one .src file to another.

Relocates the labeled data blocks for a symbol (or a file-order-inclusive span of
symbols) from a shared "temporary" file (`*_pre_data.src`, `*_sectionC.src`,
`*_sectionD.src`, `*_sectionB.src`) into the .src that should own them, keeping
`.IMPORT`/`.EXPORT` consistent in both files so the linker sees no undefined or
duplicate symbols. Sections C (const/string), D (init data) and B (bss) are all
handled by the same block model. See `.agents/skills/move-data/SKILL.md` for the
manual C-file / test half of an ownership move -- this tool only touches .src data.

    scripts/move_data.py --src A.src --dest B.src --from _sym [--to _sym] [--dry-run]

A block = a label line `_name:` plus every following line (`.DATA.*`, `.RES.*`,
`;` comments -- including the SJIS `;.SDATA` ones) up to the next label / `.SECTION`
/ `.END`. Files are read/written as raw bytes (latin-1) so Shift-JIS in comments
round-trips untouched.

Directive invariant enforced per file after the move:
  - imports == (symbols referenced in section bodies) - (symbols defined here)
  - an .EXPORT is kept iff the symbol is still defined here; the dest additionally
    exports each moved symbol that is consumed outside dest (it was exported in src,
    or src still references it after the move). Export sets can't be derived from a
    single file, so they travel with the symbol.
"""

import argparse
import difflib
import re
import sys
from pathlib import Path

IMPORT_RE = re.compile(r'^\s*\.IMPORT\s+(_[A-Za-z0-9_]+)')
EXPORT_RE = re.compile(r'^\s*\.EXPORT\s+(_[A-Za-z0-9_]+)')
SECTION_RE = re.compile(r'^\s*\.SECTION\s+(\w+)')
END_RE = re.compile(r'^\s*\.END\b')
LABEL_RE = re.compile(r'^(_[A-Za-z0-9_]+):')
# Whole identifiers; a linkable symbol is one that starts with `_` (local asm
# labels like `LAB_8c..`/`LP_GEN_..` embed an underscore but don't start with one).
IDENT_RE = re.compile(r'[A-Za-z_][A-Za-z0-9_]*')
ADDR_RE = re.compile(r'8c[0-9a-fA-F]{6}')

DEFAULT_IMPORT_PREFIX = ' ' * 10 + '.IMPORT' + ' ' * 5
DEFAULT_EXPORT_PREFIX = ' ' * 10 + '.EXPORT' + ' ' * 5
SECTION_ORDER = {'P': 0, 'C': 1, 'D': 2, 'B': 3}


def addr_key(sym):
    """Sort key from a symbol's trailing 8c-address; unaddressed sorts last."""
    m = ADDR_RE.findall(sym)
    return int(m[-1], 16) if m else 1 << 64


class Block:
    __slots__ = ('label', 'lines')

    def __init__(self, label, lines):
        self.label = label      # str, or None for a section preamble
        self.lines = lines


class Section:
    __slots__ = ('header', 'name', 'blocks')

    def __init__(self, header, name, blocks):
        self.header = header
        self.name = name
        self.blocks = blocks


class SrcFile:
    def __init__(self, path):
        self.path = Path(path)
        text = self.path.read_bytes().decode('latin-1')
        lines = text.split('\n')

        # Header region: everything before the first .SECTION / .END. Each line is
        # tagged so imports/exports can be edited while raw lines stay put.
        self.items = []  # [kind, sym|None, text]
        i = 0
        while i < len(lines) and not SECTION_RE.match(lines[i]) and not END_RE.match(lines[i]):
            line = lines[i]
            mi = IMPORT_RE.match(line)
            me = EXPORT_RE.match(line)
            if mi:
                self.items.append(['import', mi.group(1), line])
            elif me:
                self.items.append(['export', me.group(1), line])
            else:
                self.items.append(['raw', None, line])
            i += 1

        self.sections = []
        while i < len(lines) and not END_RE.match(lines[i]):
            header = lines[i]
            name = SECTION_RE.match(lines[i]).group(1)
            i += 1
            body = []
            while i < len(lines) and not SECTION_RE.match(lines[i]) and not END_RE.match(lines[i]):
                body.append(lines[i])
                i += 1
            self.sections.append(Section(header, name, split_blocks(body)))

        self.epilogue = lines[i:]  # `.END` onward, incl. trailing newline sentinel

    def render(self):
        out = [it[2] for it in self.items]
        for s in self.sections:
            out.append(s.header)
            for b in s.blocks:
                out.extend(b.lines)
        out.extend(self.epilogue)
        return '\n'.join(out)

    def defs(self):
        return {b.label for s in self.sections for b in s.blocks if b.label}

    def refs(self):
        toks = set()
        for s in self.sections:
            for b in s.blocks:
                for line in b.lines:
                    code = line.split(';', 1)[0]
                    toks.update(t for t in IDENT_RE.findall(code) if t.startswith('_'))
        return toks

    def find_section(self, name):
        for s in self.sections:
            if s.name == name:
                return s
        return None

    def exported(self):
        return {it[1] for it in self.items if it[0] == 'export'}


def split_blocks(body):
    blocks = []
    cur = Block(None, [])
    for line in body:
        m = LABEL_RE.match(line)
        if m:
            if cur.label is not None or cur.lines:
                blocks.append(cur)
            cur = Block(m.group(1), [line])
        else:
            cur.lines.append(line)
    if cur.label is not None or cur.lines:
        blocks.append(cur)
    return blocks


def directive_prefix(items, want):
    """Match the file's own column style for a new .IMPORT/.EXPORT line."""
    other = 'export' if want == 'import' else 'import'
    key = '.IMPORT' if want == 'import' else '.EXPORT'
    for it in items:
        if it[0] == want:
            return re.match(r'^(\s*\.' + want.upper() + r'\s+)', it[2]).group(1)
    for it in items:
        if it[0] == other:
            m = re.match(r'^(\s*)\.\w+(\s+)', it[2])
            return m.group(1) + key + m.group(2)
    return DEFAULT_IMPORT_PREFIX if want == 'import' else DEFAULT_EXPORT_PREFIX


def insert_directive(items, kind, sym, text):
    """Insert a new import/export item in address-sorted position among its peers."""
    a = addr_key(sym)
    pos = None
    last = -1
    for idx, it in enumerate(items):
        if it[0] == kind:
            last = idx
            if pos is None and addr_key(it[1]) > a:
                pos = idx
    if pos is None:
        if last >= 0:
            pos = last + 1
        elif kind == 'import':
            pos = next((idx for idx, it in enumerate(items) if it[0] == 'export'), len(items))
        else:
            pos = len(items)
    items.insert(pos, [kind, sym, text])


def sync_imports(f):
    """Make imports == refs - defs via minimal add/remove edits."""
    needed = f.refs() - f.defs()
    current = {it[1] for it in f.items if it[0] == 'import'}
    remove = current - needed
    f.items[:] = [it for it in f.items if not (it[0] == 'import' and it[1] in remove)]
    prefix = directive_prefix(f.items, 'import')
    for sym in sorted(needed - current, key=addr_key):
        insert_directive(f.items, 'import', sym, prefix + sym)
    return needed


def drop_undefined_exports(f):
    defs = f.defs()
    f.items[:] = [it for it in f.items if not (it[0] == 'export' and it[1] not in defs)]


def add_export(f, sym, comment):
    prefix = directive_prefix(f.items, 'export')
    insert_directive(f.items, 'export', sym, prefix + sym + comment)


def canonical_section_pos(sections, name):
    rank = SECTION_ORDER.get(name, 99)
    for idx, s in enumerate(sections):
        if SECTION_ORDER.get(s.name, 99) > rank:
            return idx
    return len(sections)


def normalize(sym):
    return sym if sym.startswith('_') else '_' + sym


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--src', required=True)
    ap.add_argument('--dest', required=True)
    ap.add_argument('--from', dest='from_sym', required=True)
    ap.add_argument('--to', dest='to_sym')
    where = ap.add_mutually_exclusive_group()
    where.add_argument('--after', help='insert after this symbol in the dest section')
    where.add_argument('--before', help='insert before this symbol in the dest section')
    ap.add_argument('--dry-run', action='store_true')
    args = ap.parse_args()

    from_sym = normalize(args.from_sym)
    to_sym = normalize(args.to_sym) if args.to_sym else from_sym

    src = SrcFile(args.src)
    dest = SrcFile(args.dest)
    src_before = src.render()
    dest_before = dest.render()

    # Locate the inclusive span: from_sym .. to_sym must be labels in one section.
    sec = i = j = None
    for s in src.sections:
        labels = [b.label for b in s.blocks]
        if from_sym in labels:
            sec = s
            i = labels.index(from_sym)
            if to_sym not in labels:
                sys.exit(f'error: --from {from_sym} and --to {to_sym} are not in the '
                         f'same section of {src.path.name}')
            j = labels.index(to_sym)
            break
    if sec is None:
        sys.exit(f'error: symbol {from_sym} not found as a data label in {src.path.name}')
    if j < i:
        sys.exit(f'error: --to {to_sym} precedes --from {from_sym} in file order')

    moved = sec.blocks[i:j + 1]
    M = [b.label for b in moved]

    # A data block should start at a 4-aligned address; a misaligned start is a sign
    # the boundary was drawn at the wrong place (wrong decompilation). A misaligned
    # *end* is fine -- the linker realigns the next section -- so size isn't checked.
    # Check the blocks the cut touches: the moved ones, and the block the cut leaves
    # exposed at the seam in the origin.
    seam = sec.blocks[j + 1] if j + 1 < len(sec.blocks) else None
    for b in moved + ([seam] if seam else []):
        if b.label and addr_key(b.label) % 4:
            print(f'WARNING: {b.label} starts at a non-4-aligned address -- likely a '
                  f'wrong span boundary / mis-decompiled object', file=sys.stderr)

    # Remember which moved symbols src exported, with their consumer comments.
    src_export_comment = {}
    for it in src.items:
        if it[0] == 'export' and it[1] in M:
            src_export_comment[it[1]] = it[2].split(it[1], 1)[1]

    # 1. Detach from src.
    del sec.blocks[i:j + 1]

    # 2. Attach to dest: next to --after/--before anchor, else at the matching
    #    section's end (creating the section in canonical order if absent).
    anchor = normalize(args.after or args.before) if (args.after or args.before) else None
    if anchor:
        dsec, k = None, None
        for s in dest.sections:
            labels = [b.label for b in s.blocks]
            if anchor in labels:
                dsec, k = s, labels.index(anchor)
                break
        if dsec is None:
            sys.exit(f'error: anchor {anchor} not found in {dest.path.name}')
        if dsec.name != sec.name:
            sys.exit(f'error: anchor {anchor} is in section {dsec.name}, but the moved '
                     f'data is section {sec.name}')
        at = k + 1 if args.after else k
        dsec.blocks[at:at] = moved
    else:
        dsec = dest.find_section(sec.name)
        if dsec is None:
            pos = canonical_section_pos(dest.sections, sec.name)
            dest.sections.insert(pos, Section(sec.header, sec.name, list(moved)))
        else:
            dsec.blocks.extend(moved)

    # 3. Reconcile directives.
    src_needed = sync_imports(src)
    drop_undefined_exports(src)
    sync_imports(dest)

    dest_exported = dest.exported()
    for sym in M:
        if sym in dest_exported:
            continue
        if sym in src_export_comment:
            comment = src_export_comment[sym]
        elif sym in src_needed:              # src still references it -> src is a consumer
            comment = ' ; ' + src.path.name
        else:
            continue                          # now private to dest
        add_export(dest, sym, comment)

    src_after = src.render()
    dest_after = dest.render()

    if args.dry_run:
        for label, before, after in ((src.path, src_before, src_after),
                                     (dest.path, dest_before, dest_after)):
            diff = difflib.unified_diff(before.splitlines(), after.splitlines(),
                                        f'a/{label}', f'b/{label}', lineterm='')
            sys.stdout.write('\n'.join(diff) + '\n')
    else:
        src.path.write_bytes(src_after.encode('latin-1'))
        dest.path.write_bytes(dest_after.encode('latin-1'))

    print(f'moved {len(M)} block(s) [{M[0]} .. {M[-1]}] '
          f'from {src.path.name} to {dest.path.name} (section {sec.name})')
    print('note: group-marker comments (lone `; <addr>` lines) stay in the source '
          'file; review export consumer comments and re-run the matching build + '
          'scripts/dcdiff.py to confirm the binary is unchanged.')


if __name__ == '__main__':
    main()
