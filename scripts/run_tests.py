#!/usr/bin/env python3
"""Build the test objects and run the suite.

Usage: run_tests.py [sh4objtest suite options] [test file]
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build"


def lock(f):
    """Block until we hold an exclusive lock on f; released when f closes."""
    if os.name == "nt":
        import msvcrt
        while True:
            try:
                msvcrt.locking(f.fileno(), msvcrt.LK_LOCK, 1)
                return
            except OSError:  # LK_LOCK gives up after ~10s
                pass
    else:
        import fcntl
        fcntl.flock(f, fcntl.LOCK_EX)


def main():
    tools = os.pathsep.join([str(BUILD / "tools"), os.environ.get("PATH", "")])
    sh4objtest = shutil.which("sh4objtest", path=tools)
    if not sh4objtest:
        sys.exit("sh4objtest not found; run scripts/install_tools.py")

    os.chdir(ROOT)
    BUILD.mkdir(exist_ok=True)
    # `build.py test` wipes build/output_test, so two concurrent runs delete
    # each other's objects mid-build. Serialize instead of racing.
    with open(BUILD / ".test.lock", "a") as f:
        lock(f)
        build = subprocess.run([sys.executable, "build.py", "test"])
        if build.returncode:
            sys.exit(build.returncode)
        suite = subprocess.run([sh4objtest, "suite", "-s", "tests.php", *sys.argv[1:]])
    sys.exit(suite.returncode)


if __name__ == "__main__":
    main()
