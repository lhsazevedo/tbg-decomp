#!/usr/bin/env python3
import json, os, sys, pathlib

data  = json.load(sys.stdin)
event = data.get('hook_event_name', '')
tool  = data.get('tool_name', '')
tip   = data.get('tool_input', {})
file_str = tip.get('file_path', '')

if not file_str:
    sys.exit(0)

file = pathlib.Path(file_str)
if not file.is_absolute():
    file = (pathlib.Path(os.getcwd()) / file).resolve()
else:
    file = file.resolve()

script_dir   = pathlib.Path(__file__).resolve().parent
project_root = script_dir.parent.parent

try:
    rel = file.relative_to(project_root)
    if rel.parts[0] not in ('src', 'tests'):
        sys.exit(0)
except ValueError:
    sys.exit(0)

BACKUP_SUFFIX = '.claude_sjis_orig'
backup = pathlib.Path(str(file) + BACKUP_SUFFIX)


def deny(msg):
    print(json.dumps({
        'hookSpecificOutput': {
            'hookEventName': 'PreToolUse',
            'permissionDecision': 'deny',
            'permissionDecisionReason': msg,
        }
    }))
    sys.exit(0)


def has_non_ascii(path):
    return any(b > 0x7F for b in path.read_bytes())


if event == 'PreToolUse' and tool == 'Read':
    if not file.exists() or not has_non_ascii(file):
        sys.exit(0)
    raw = file.read_bytes()
    backup.write_bytes(raw)
    file.write_bytes(raw.decode('shift_jis').encode('utf-8'))
    sys.exit(0)  # Allow Read to proceed on the now-UTF-8 file

elif event == 'PostToolUse' and tool == 'Read':
    if backup.exists():
        backup.rename(file)
    sys.exit(0)

elif event == 'PreToolUse' and tool == 'Write':
    content = tip.get('content', '')
    file.parent.mkdir(parents=True, exist_ok=True)
    file.write_bytes(content.encode('shift_jis'))
    deny(f"Write intercepted: content saved as Shift-JIS at {file}")

elif event == 'PreToolUse' and tool == 'Edit':
    if not file.exists():
        sys.exit(0)
    old         = tip.get('old_string', '')
    new         = tip.get('new_string', '')
    replace_all = tip.get('replace_all', False)

    text   = file.read_bytes().decode('shift_jis')
    result = text.replace(old, new) if replace_all else text.replace(old, new, 1)

    if result == text:
        print(f"old_string not found in {file}", file=sys.stderr)
        sys.exit(1)

    file.write_bytes(result.encode('shift_jis'))
    deny(f"Edit intercepted: substitution applied in UTF-8, file saved as Shift-JIS at {file}")
