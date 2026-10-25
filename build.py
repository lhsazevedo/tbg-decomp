#!/usr/bin/env python3
"""Build the game or the unit-test objects.

targets:
  regular   the game from C, into build/output/tbg.bin (default)
  matching  the game from the archived asm, checked against the original binary
  test      every unit's C and archived asm, for scripts/run_tests.sh
  clean     remove every target's output
"""
import argparse
import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BUILD = Path("build")

# Link order
UNITS = [
    "010080_main",
    "0100bc_sound",
    "010e90_vibration",
    "010fe8_heap",
    "011120_asset_queues",
    "012324_input",
    "0129cc_game",
    "013ae8_route",
    "014934_unused_load",
    "0149b0_sbinit",
    "1ba1c8_globals",
    "014a9c_tasks",
    "014b8c_backup",
    "014f54_sprite",
    "015034_text",
    "015ab8_title",
    "016108_resgrp_free",
    "01614c_replay_menu",
    "016c58_prompt",
    "016d2c_course_menu",
    "018644_file_select",
    "0193c8_vm_select",
    "019e98_main_menu",
    "01a148_option",
    "01b19c_system_menu",
    "01bb48_vm_game",
    "01c980_profile_file",
    "01d290_album",
    "01d7fc_results",
    "01e27c_practice_menu",
    "01f3c0_ending",
    "01fa78_hud",
    "020214_drive_cue",
    "020594_vehicle_model",
    "0206f0_intersect",
    "0207d4_vec_xz",
    "02081c_geom",
    "020914_ground_query",
    "020b6c_ground_probe",
    "02171c_tile_stream",
    "021b9c_tile_draw",
    "022464_render",
    "022bdc_bus",
    "023310_bus_init",
    "023938_bus_drive",
    "02412c_bus_line",
    "024280_bus_input",
    "024b4c_bus_camera",
    "025870_cinematic",
    "025b98_traffic_drive",
    "026710_traffic",
    "02786c_vehicle_parts",
    "027958_bus_draw",
    "028258_signal",
    "0289ac_objects",
    "02a9fc_message_box",
    "02af78_event",
    "02b2f0_drive_msg",
    "02b464_grading",
    "02c884_stop",
    "02d06c_stop_draw",
    "02d19c_passenger",
    "02d968_stop_spawn",
    "02df3c_traffic_lookahead",
    "02e2dc_bus_collision",
    "02e400_collision",
    "02e51c_attr_query",
    "02f0c8_traffic_path_scan",
    "02f320_replay_codec",
    "scif",
    "serial_debug",
    "04ce10_line_nodes",
    "235ca0_nj_buffers",
]

# Not in the original game
EXTRA_UNITS = {"scif", "serial_debug"}

DECOMPILED_ASM_DIR = Path("src/asm/decompiled")
ORIGINAL_SHA1 = "a6df9e0de39b2d11e9339aef915d20e35763ec81"
ASMSH_FLAGS = ["-debug", "-cpu=sh4", "-endian=little", "-sjis"]

# shc/asmsh `file(line) : NNNN (W) message`
COMPILER_MESSAGE_REGEX = re.compile(rb"\(\d+\) : \d+ \([IW]\) ")
# lnk `** NNN MESSAGE`
LINKER_MESSAGE_REGEX = re.compile(rb"^\*\* ")


def find_tool(name):
    """Prefer the copy scripts/install_tools.py downloaded."""
    local = BUILD / "tools" / name
    if local.exists():
        return str(local)
    found = shutil.which(name)
    if not found:
        sys.exit(f"{name} not found; run scripts/install_tools.py")
    return found


class Wibo:
    """Runs the Windows SDK tools through wibo."""

    def __init__(self):
        self.wibo = find_tool("wibo")

    def paths(self, *paths):
        """Host paths as the SDK tools see them."""
        out = subprocess.run([self.wibo, "path", "-w", *map(str, paths)],
                             capture_output=True, text=True, check=True)
        return out.stdout.splitlines()

    def command(self, exe):
        return [self.wibo, str(exe)]


class Windows:
    """Runs the SDK tools directly."""

    def paths(self, *paths):
        return [str(p) for p in paths]

    def command(self, exe):
        return [str(exe)]


PLATFORMS = {"Linux": Wibo, "Darwin": Wibo, "Windows": Windows}


class Toolchain:
    def __init__(self, host, verbose, listing):
        dc_sdk = os.environ.get("DC_SDK")
        if not dc_sdk:
            sys.exit("DC_SDK is not set; point it at the SDK")
        # lnk rejects '-' in paths; relative to the repo, none above it count
        try:
            self.root = Path(os.path.relpath(dc_sdk))
        except ValueError:  # Windows, SDK on another drive
            self.root = Path(os.path.abspath(dc_sdk))
        if "-" in str(self.root):
            sys.exit(f"SDK path {self.root} contains '-', which lnk rejects; "
                     "rename it or symlink the SDK to a path without one")
        self.host = host
        self.verbose = verbose
        self.listing = listing
        self.warnings = 0
        tmp = BUILD / "tmp"
        tmp.mkdir(parents=True, exist_ok=True)
        self.win_root, win_tmp = host.paths(self.root, tmp)
        p = lambda sub: self.win_root + "\\" + sub
        self.env = dict(
            os.environ,
            SHINOBI_DIR=p("shinobi"),
            KATANA_SDK_DIR=self.win_root,
            SHC_BIN=p("shc\\bin"),
            SHC_LIB=p("shc\\bin"),
            SHC_INC=p("shc\\include") + "," + p("shinobi\\include"),
            SHC_TMP=win_tmp,
        )

    def run(self, label, exe, *args):
        """Show only the tool's messages, unless it fails. A pass with no
        `label` repeats an earlier one's messages, so shows none."""
        cmd = self.host.command(self.root / exe) + list(args)
        if self.verbose:
            print(" ".join(cmd), flush=True)
        elif label:
            print(label, flush=True)
        result = subprocess.run(cmd, env=self.env,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        out = sys.stdout.buffer
        if self.verbose or result.returncode != 0:
            out.write(result.stdout)
            out.flush()
            if result.returncode != 0:
                sys.exit(f"{Path(exe).stem} failed")
            return
        if not label:
            return
        prev = b""
        for line in result.stdout.replace(b"\r", b"").splitlines():
            if COMPILER_MESSAGE_REGEX.search(line) or LINKER_MESSAGE_REGEX.match(line):
                # lnk runs each subcommand as it echoes it
                if prev.startswith(b": "):
                    out.write(prev + b"\n")
                out.write(line + b"\n")
                if b"(I)" not in line:
                    self.warnings += 1
            prev = line
        out.flush()

    def summary(self):
        if self.warnings:
            print(f"{self.warnings} warning{'s' if self.warnings > 1 else ''}")

    def asmsh(self, src, obj, defines=()):
        flags = [f"-define={d}=1" for d in defines] + ASMSH_FLAGS
        self.run(f"asmsh {src}", "shc/bin/asmsh.exe", backslashes(src),
                 f"-object={backslashes(obj)}", *flags)

    def shc(self, src, obj, sub, defines=()):
        flags = [f"-sub={sub}"]
        if defines:
            flags.append("-define=" + ",".join(defines))
        self.run(f"shc {src}", "shc/bin/shc.exe", backslashes(src),
                 f"-object={backslashes(obj)}", *flags)
        if self.listing:
            self.run(None, "shc/bin/shc.exe", backslashes(src), "-code=asm",
                     f"-object={backslashes(obj)}.src", *flags)

    def link(self, objs, template, sub):
        inputs = "".join(f"input {backslashes(o)}\n" for o in objs)
        text = template.read_text()
        text = text.replace("@DC_SDK@", self.win_root)
        sub.write_text(text.replace("@INPUTS@\n", inputs))
        self.run(f"lnk {sub}", "shc/bin/lnk.exe", f"-sub={backslashes(sub)}")

    def elf2bin(self, elf):
        self.run(f"elf2bin {elf}", "bin/elf2bin.exe", "-s", "8c010000", backslashes(elf))


def backslashes(p):
    return str(p).replace("/", "\\")


def fresh(out):
    shutil.rmtree(out, ignore_errors=True)


def obj_path(out, src):
    obj = out / src.with_suffix(".obj")
    obj.parent.mkdir(parents=True, exist_ok=True)
    return obj


def build_regular(sdk, args):
    out = BUILD / "output"
    fresh(out)
    defines = ["__SHC__"]
    if args.serial_debug:
        defines.append("SERIAL_DEBUG")
    if args.debug_menu:
        defines.append("DEBUG_MENU")
    if args.lang == "en":
        defines.append("GAME_LANG_EN")
    if args.log_level:
        defines.append(f"DEBUG_LEVEL=LOG_LEVEL_{args.log_level}")

    objs = []
    for unit in UNITS:
        src = Path("src") / f"{unit}.c"
        obj = obj_path(out, src)
        sdk.shc(src, obj, BUILD / "shc.sub", defines)
        objs.append(obj)
    sdk.link(objs, BUILD / "lnk_template.sub", out / "lnk.sub")
    sdk.elf2bin(out / "tbg.elf")
    sdk.summary()
    banner("Project built :)")


def build_matching(sdk, args):
    out = BUILD / "output_matching"
    fresh(out)
    objs = []
    for unit in UNITS:
        if unit in EXTRA_UNITS:
            continue
        src = DECOMPILED_ASM_DIR / f"{unit}.src"
        if src.exists():
            obj = obj_path(out, src)
            sdk.asmsh(src, obj, ["MATCHING"])
        else:
            src = Path("src") / f"{unit}.c"
            obj = obj_path(out, src)
            sdk.shc(src, obj, BUILD / "shc_matching.sub")
        objs.append(obj)
    sdk.link(objs, BUILD / "lnk_matching_template.sub", out / "lnk.sub")
    sdk.elf2bin(out / "tbg.elf")
    sdk.summary()

    sha1 = hashlib.sha1((out / "tbg.bin").read_bytes()).hexdigest()
    if sha1 != ORIGINAL_SHA1:
        banner("Oops, build differs :/")
        sys.exit(1)
    banner("Yay! Build matches :)")


def build_test(sdk, args):
    """A unit the matching build compiles from C has no archive and is tested
    against the C alone."""
    out = BUILD / "output_test"
    fresh(out)
    for src in sorted(DECOMPILED_ASM_DIR.glob("*.src")):
        sdk.asmsh(src, obj_path(out, src), ["UNIT_TESTING"])
    for src in sorted(Path("src").glob("*.c")):
        sdk.shc(src, obj_path(out, src), BUILD / "shc_testing.sub")
    sdk.summary()


def banner(msg):
    line = "=" * len(msg)
    print(f"{line}\n{msg}\n{line}")


def clean():
    for out in ("output", "output_matching", "output_test"):
        fresh(BUILD / out)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("target", nargs="?", default="regular",
                        choices=["regular", "matching", "test", "clean"],
                        metavar="target", help="see targets above")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="show the full tool commands and their output")
    parser.add_argument("--listing", action="store_true",
                        help="also write each C unit's compiled asm to <obj>.src")
    regular = parser.add_argument_group("regular build options")
    regular.add_argument("--serial-debug", action="store_true",
                       help="enable serial logging")
    regular.add_argument("--log-level", metavar="LEVEL",
                       help="override the serial log level (default INFO); "
                            "see the LOG_LEVEL_* constants")
    regular.add_argument("--debug-menu", action="store_true",
                       help="boot straight into the unused debug menu")
    regular.add_argument("--lang", choices=["ja", "en"], default="ja",
                       help="game language (default: ja, the original)")
    args = parser.parse_args()

    if args.target != "regular" and (args.serial_debug or args.log_level
                                   or args.debug_menu or args.lang != "ja"):
        parser.error("build options only apply to the regular build")

    os.chdir(ROOT)
    if args.target == "clean":
        clean()
        return
    sdk = Toolchain(PLATFORMS[platform.system()](), args.verbose, args.listing)
    {"regular": build_regular, "matching": build_matching, "test": build_test}[args.target](sdk, args)


if __name__ == "__main__":
    main()
