# Copyright 2026 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
from packaging.version import Version
import configparser
import logging
import math
import os
import sys
import testenv
import traceback

cfg = {
    "max_jobs_per_gb_ram": None,
    "titan_min": "11.1.0",
}

titan_version_in_path = None


def get_titan_make_job_count():
    max_jobs_per_gb_ram = cfg["max_jobs_per_gb_ram"]
    max_jobs = None

    if max_jobs_per_gb_ram:
        try:
            gb_ram = 0
            with open("/proc/meminfo") as f:
                line = f.readline()
                # Parse e.g. "MemTotal:       15571604 kB"
                if line.startswith("MemTotal:"):
                    gb_ram = int(line.split(" ")[-2]) / 1024 / 1024
                    logging.debug(f"Building with {round(gb_ram, 2)} GB of RAM")
            max_jobs = math.floor(gb_ram * float(max_jobs_per_gb_ram))
            if max_jobs < 1:
                raise RuntimeError(f"max_jobs is invalid: max_jobs={max_jobs}, gb_ram={gb_ram}")

        except Exception as ex:
            traceback.print_exception(type(ex), ex, ex.__traceback__)
            logging.error(f"Calculating max jobs with max_jobs_per_gb_ram={max_jobs_per_gb_ram} failed, assuming 4")
            max_jobs = 4

    if max_jobs and max_jobs < testenv.args.jobs:
        logging.info(
            f"Using only {max_jobs} jobs instead of {testenv.args.jobs} because of"
            f" max_jobs_per_gb_ram={max_jobs_per_gb_ram} in testsrcdir.cfg"
        )
        return max_jobs

    return testenv.args.jobs


def get_titan_version_in_path():
    global titan_version_in_path

    if titan_version_in_path:
        return titan_version_in_path

    ret = None
    v = testenv.cmd.run(["ttcn3_compiler", "-v"], capture_output=True, text=True)
    for line in v.stderr.split("\n"):
        if line.startswith("Version: "):
            ret = line.split(":", 1)[1].strip()
            logging.debug(f"eclipse-titan version: {ret}")
            break
    titan_version_in_path = ret
    return ret


def get_titan_version():
    ret_version = cfg["titan_min"]
    ret_reason = "from titan_min= in testsrcdir.cfg"

    if testenv.args.titan_version:
        if Version(ret_version) > Version(testenv.args.titan_version):
            logging.error(
                f"--titan-version={testenv.args.titan_version} is lower than titan_min={ret_version} in testsrcdir.cfg"
            )
            sys.exit(1)
        ret_version = testenv.args.titan_version
        ret_reason = "from --titan-version"

    if not testenv.args.podman and not os.path.exists(f"/opt/eclipse-titan-{ret_version}"):
        path_version = get_titan_version_in_path()
        if not path_version:
            logging.error("Failed to parse the ttcn3_compiler version.")
            logging.error(f"Install eclipse-titan {ret_version} or higher or use --podman.")
            sys.exit(1)
        if testenv.args.titan_version and ret_version != path_version:
            logging.error(
                f"Installed eclipse-titan version {path_version} is not the same as --titan-version={ret_version}."
            )
            sys.exit(1)
        if Version(ret_version) > Version(path_version):
            logging.error(
                f"Installed eclipse-titan version {path_version} is lower than titan_min={ret_version} in testsrcdir.cfg."
            )
            logging.error(f"Install eclipse-titan {ret_version} or higher or use --podman.")
            sys.exit(1)
        ret_version = path_version
        ret_reason = "installed on host system"

    return ret_version, ret_reason


def init():
    global cfg

    cfg_path = os.path.join(testenv.ttcn3_hacks_dir, testenv.args.testsuite, "testsrcdir.cfg")
    if not os.path.exists(cfg_path):
        return

    parser = configparser.ConfigParser()
    parser.read(cfg_path)

    if "testsrcdir" not in parser:
        logging.error(f"Missing section [testsrcdir] in {cfg_path}")
        sys.exit(1)

    for section in parser:
        if section == "DEFAULT":
            continue
        if section != "testsrcdir":
            logging.error(f"Invalid section [{section}] in {cfg_path}")
            sys.exit(1)

    for key in parser["testsrcdir"]:
        if key not in cfg:
            logging.error("Invalid key {key}= in {cfg_path}")
            sys.exit(1)
        cfg[key] = parser["testsrcdir"][key]

    get_titan_version()
