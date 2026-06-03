# Copyright 2026 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import configparser
import logging
import math
import os
import sys
import testenv
import traceback

cfg = {
    "max_jobs_per_gb_ram": None,
}


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
