# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import datetime
import fnmatch
import json
import logging
import os
import shlex
import shutil
import subprocess
import testenv
import testenv.daemons
import testenv.testdir
import urllib
import urllib.request

executable_path = None

lxc_netdev = "eth0"
lxc_ip_pattern = "10.0.*"
lxc_port = 8042


def find_lxc_host_ip():
    cmd = ["ip", "-j", "-o", "-4", "addr", "show", "dev", lxc_netdev]
    p = testenv.cmd.run(cmd, check=False, no_podman=True, capture_output=True, text=True)
    ret = json.loads(p.stdout)[0]["addr_info"][0]["local"]
    if fnmatch.fnmatch(ret, lxc_ip_pattern):
        ret = ret.split(".")
        ret = f"{ret[0]}.{ret[1]}.{ret[2]}.1"
        return ret
    return None


def get_from_coredumpctl_lxc_host():
    # Server implementation: osmo-ci, ansible/roles/testenv-coredump-helper
    global executable_path

    logging.info("Looking for a coredump on lxc host")

    ip = os.environ.get("TESTENV_COREDUMP_FROM_LXC_HOST_IP") or find_lxc_host_ip()
    if not ip:
        logging.warning("Failed to get lxc host ip, can't look for coredump")
        return

    try:
        with urllib.request.urlopen(f"http://{ip}:{lxc_port}/core") as response:
            executable_path = dict(response.getheaders())["X-Executable-Path"]
            with open(f"{testenv.testdir.testdir}/core", "wb") as h:
                shutil.copyfileobj(response, h)
            logging.debug("Coredump found and copied to log dir")
    except urllib.error.HTTPError as e:
        executable_path = None
        if e.code == 404:
            logging.debug("No coredump found")
        else:
            logging.error(f"Unexpected error while attempting to fetch the coredump: {e}")
    except urllib.error.URLError as e:
        executable_path = None
        logging.error(f"Unexpected error while attempting to fetch the coredump: {e}")


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


def get_from_coredumpctl_local():
    global executable_path

    logging.info("Looking for a coredump")

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


def get_from_coredumpctl():
    if os.environ.get("TESTENV_COREDUMP_FROM_LXC_HOST"):
        get_from_coredumpctl_lxc_host()
    else:
        get_from_coredumpctl_local()


def get_backtrace():
    global executable_path

    core_path = f"{testenv.testdir.testdir}/core"
    if not executable_path or not os.path.exists(core_path):
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
