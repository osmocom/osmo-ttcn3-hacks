# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import datetime
import fnmatch
import glob
import json
import logging
import re
import shlex
import shutil
import subprocess
import testenv
import testenv.daemons
import testenv.testdir

core_path = None
executable_path = None


def executable_is_relevant(exe):
    if testenv.args.binary_repo:
        patterns = [
            "*/usr/bin/open5gs-*",
            "*/usr/bin/osmo-*",
        ]

        for pattern in patterns:
            if fnmatch.fnmatch(exe, pattern):
                return True
    else:
        if exe.startswith(testenv.args.cache):
            return True

    return False


def get_from_coredumpctl():
    global core_path
    global executable_path

    logging.info("Looking for a coredump with coredumpctl")

    if not shutil.which("coredumpctl"):
        logging.debug("coredumpctl is not available, won't try to get coredump")
        return

    # Check for any coredump within last 3 seconds
    since = (datetime.datetime.now() - datetime.timedelta(seconds=3)).strftime("%Y-%m-%d %H:%M:%S")
    cmd = ["coredumpctl", "-q", "-S", since, "--json=short", "-n1"]
    logging.debug(f"+ {cmd}")

    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode != 0:
        logging.debug("No coredump found")
        return

    # Check if the coredump executable is from osmo-*, open5gs-*, etc.
    coredump = json.loads(p.stdout)[0]
    if not executable_is_relevant(coredump["exe"]):
        logging.debug("Found an unrelated coredump, ignoring")
        return

    logging.debug("Coredump found, copying to log dir")
    core_path = f"{testenv.testdir.testdir}/core"
    testenv.cmd.run(
        ["coredumpctl", "dump", "-q", "-S", since, "-o", core_path, str(coredump["pid"]), coredump["exe"]],
        stdout=subprocess.DEVNULL,
        no_podman=True,
    )

    executable_path = coredump["exe"]


def get_from_file():
    global core_path
    global executable_path

    logging.info("Looking for a coredump file")

    glob_matches = glob.glob(f"{testenv.testdir.testdir}/*/core*")
    if not glob_matches:
        return

    core_path = glob_matches[0]
    p = testenv.cmd.run(
        ["file", core_path],
        no_podman=True,
        capture_output=True,
        text=True,
    )
    print(p.stdout, end="")

    execfn_match = re.search(r"execfn: '(.*?)'", p.stdout)
    if execfn_match:
        executable_path = execfn_match.group(1)
        logging.debug("Coredump file and execfn found")
    else:
        logging.debug("Failed to get execfn path from coredump file")


def get_coredump():
    with open("/proc/sys/kernel/core_pattern") as f:
        pattern = f.readline().rstrip()

    if "systemd-coredump" in pattern:
        get_from_coredumpctl()
    elif pattern.startswith("core"):
        get_from_file()
    else:
        logging.debug(f"Unsupported core_pattern, won't try to get coredump: {pattern}")


def get_backtrace():
    global executable_path

    if not executable_path or not core_path:
        return

    logging.info("Running gdb to get a backtrace")

    cmd = "gdb"
    cmd += " --batch"
    cmd += f" {shlex.quote(executable_path)}"
    cmd += f" {shlex.quote(core_path)}"
    cmd += " -ex bt"
    cmd += f" | tee {shlex.quote(core_path)}.backtrace"

    testenv.cmd.run(cmd)

    executable_path = None
