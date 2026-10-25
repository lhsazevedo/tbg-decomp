#!/usr/bin/env python3
"""Download the build tools at the versions pinned below into build/tools/,
which should be on PATH.

Usage: install_tools.py [tool ...]   (default: all)

Tools already at their pinned version are skipped, so re-run this after
changing a version.
"""
import io
import platform
import shutil
import sys
import tarfile
import urllib.request
import zipfile
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent.parent / "build" / "tools"

# tool: (version, {platform: download URL}); {v} in a URL becomes the version
TOOLS = {
    "wibo": ("1.1.0", {
        "linux-x86_64": "https://github.com/decompals/wibo/releases/download/{v}/wibo-x86_64",
        "darwin-x86_64": "https://github.com/decompals/wibo/releases/download/{v}/wibo-macos",
        "darwin-arm64": "https://github.com/decompals/wibo/releases/download/{v}/wibo-macos",
    }),
    "sh4objtest": ("0.1.48", {
        "linux-x86_64": "https://github.com/lhsazevedo/sh4objtest/releases/download/v{v}/sh4objtest-linux-x86_64.tar.gz",
    }),
    "gdworkbench": ("1.0.0-beta.5", {
        "linux-x86_64": "https://github.com/lhsazevedo/gdworkbench/releases/download/v{v}/gdworkbench_{v}_linux_amd64.tar.gz",
        "linux-arm64": "https://github.com/lhsazevedo/gdworkbench/releases/download/v{v}/gdworkbench_{v}_linux_arm64.tar.gz",
        "darwin-x86_64": "https://github.com/lhsazevedo/gdworkbench/releases/download/v{v}/gdworkbench_{v}_darwin_amd64.tar.gz",
        "darwin-arm64": "https://github.com/lhsazevedo/gdworkbench/releases/download/v{v}/gdworkbench_{v}_darwin_arm64.tar.gz",
        "windows-x86_64": "https://github.com/lhsazevedo/gdworkbench/releases/download/v{v}/gdworkbench_{v}_windows_amd64.zip",
    }),
}


def current_platform():
    system = platform.system().lower()
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "aarch64": "arm64"}.get(machine, machine)
    return f"{system}-{machine}"


def extract(data, url, name):
    """Return the executable, unpacking `name` if the download is an archive."""
    if url.endswith(".tar.gz"):
        with tarfile.open(fileobj=io.BytesIO(data)) as tar:
            return tar.extractfile(name).read()
    if url.endswith(".zip"):
        with zipfile.ZipFile(io.BytesIO(data)) as z:
            return z.read(name)
    return data


def install(tool, plat):
    version, urls = TOOLS[tool]
    exe = TOOLS_DIR / (tool + ".exe" if plat.startswith("windows") else tool)
    stamp = TOOLS_DIR / (tool + ".version")
    if exe.exists() and stamp.exists() and stamp.read_text().strip() == version:
        return True
    if plat not in urls:
        print(f"{tool}: no {plat} release, skipped", file=sys.stderr)
        return True

    url = urls[plat].format(v=version)
    print(f"{tool} {version}: {url}")
    with urllib.request.urlopen(url) as resp:
        data = resp.read()
    TOOLS_DIR.mkdir(parents=True, exist_ok=True)
    tmp = exe.with_name(exe.name + ".tmp")
    tmp.write_bytes(extract(data, url, exe.name))
    tmp.chmod(0o755)
    shutil.move(tmp, exe)
    stamp.write_text(version + "\n")
    return True


def main():
    tools = sys.argv[1:] or list(TOOLS)
    unknown = [t for t in tools if t not in TOOLS]
    if unknown:
        sys.exit(f"unknown tool(s): {', '.join(unknown)}; known: {', '.join(TOOLS)}")
    plat = current_platform()
    ok = [install(t, plat) for t in tools]
    sys.exit(0 if all(ok) else 1)


if __name__ == "__main__":
    main()
