# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import datetime
import glob
import logging
import os
import os.path
import tempfile
import testenv
import testenv.cmd
import testenv.testsuite
import uuid


# Some testsuites have multiple configurations (like bts: generic, hopping, …).
# For each configuration, prepare() gets called. testdir is the one for the
# current configuration.
testdir = None
testdir_topdir = None
prefix = None
clean_scripts = {}


def init():
    global testdir_topdir
    global prefix

    prefix = f"testenv-{testenv.args.testsuite}-"
    if testenv.args.config:
        prefix += f"{'-'.join(testenv.args.config).replace('*', '')}-"
    if testenv.args.binary_repo:
        prefix += f"{testenv.args.binary_repo.replace(':', '-')}-"
    prefix += datetime.datetime.now().strftime("%Y%m%d-%H%M")
    prefix += f"-{str(uuid.uuid4()).split('-', 1)[0]}"

    if testenv.args.log_dir:
        testdir_topdir = testenv.args.log_dir
        os.makedirs(testdir_topdir)
    else:
        testdir_topdir = tempfile.mkdtemp(prefix=f"{prefix}-")

    atexit.register(clean)

    logging.info(f"Logging to: {testdir_topdir}")

    # Add a convenience symlink
    if not testenv.args.log_dir:
        if os.path.exists("/tmp/logs"):
            testenv.cmd.run(["rm", "/tmp/logs"], no_podman=True)
        testenv.cmd.run(["ln", "-sf", testdir_topdir, "/tmp/logs"], no_podman=True)


def prepare(cfg_name, cfg, loop_count):
    global testdir
    global clean_scripts

    topdir = testdir_topdir

    if testenv.args.until_nok:
        topdir = os.path.join(topdir, f"loop-{loop_count}")

    if len(testenv.testenv_cfg.cfgs) == 1:
        testdir = topdir
    else:
        testdir = os.path.join(topdir, cfg_name.replace("testenv_", "").replace(".cfg", ""))

    logging.info(f"Preparing testdir: {testdir}")
    testsuite_dir = os.path.join(testenv.testsuite.ttcn3_hacks_dir, testenv.args.testsuite)

    atexit.register(clean_run_scripts)

    for section in cfg:
        if section in ["DEFAULT"]:
            continue

        section_data = cfg[section]
        section_dir = os.path.join(testdir, section)
        os.makedirs(section_dir)

        if "config" in section_data and section == "testsuite":
            file = section_data["config"]
            path = os.path.join(testsuite_dir, file)
            path_dest = os.path.join(section_dir, file)
            testenv.cmd.run(["install", "-Dm644", path, path_dest])

        if "copy" in section_data:
            sources = section_data["copy"].split(" ")
            testenv.cmd.run(["cp", "-a"] + sources + [section_dir], no_podman=True, cwd=testsuite_dir)

    # Referenced in testsuite cfgs: *.default
    pattern = os.path.join(testsuite_dir, "*.default")
    for path in glob.glob(pattern):
        path_dest = os.path.join(testdir, "testsuite", os.path.basename(path))
        testenv.cmd.run(["install", "-Dm644", path, path_dest])

    # Referenced in testsuite cfgs: Common.cfg
    common_cfg = os.path.join(testdir, "testsuite", "Common.cfg")
    path = os.path.join(testenv.testsuite.ttcn3_hacks_dir, "Common.cfg")
    testenv.cmd.run(["install", "-Dm644", path, common_cfg])
    testenv.cmd.run(
        [
            "sed",
            "-i",
            f's#TTCN3_HACKS_PATH := .*#TTCN3_HACKS_PATH := "{testenv.testsuite.ttcn3_hacks_dir}"#',
            common_cfg,
        ]
    )

    # Adjust testsuite config: set mp_osmo_repo, set Common.cfg path in the
    # testsuite's config and in all configs it may include
    mp_osmo_repo = "latest" if testenv.args.latest else "nightly"
    line = f'Misc_Helpers.mp_osmo_repo := "{mp_osmo_repo}"'

    patterns = [
        os.path.join(testdir, "testsuite/**/*.cfg"),
        os.path.join(testdir, "testsuite/**/*.default"),
    ]
    for pattern in patterns:
        for cfg_file in glob.glob(pattern, recursive=True):
            logging.debug(f"Adjusting testsuite config: {cfg_file}")
            testenv.cmd.run(
                [
                    "sed",
                    "-i",
                    "-e",
                    f"s/\\[MODULE_PARAMETERS\\]/\\[MODULE_PARAMETERS\\]\\n{line}/g",
                    "-e",
                    "s#../Common.cfg#Common.cfg#",
                    cfg_file,
                ]
            )

    # Run prepare and clean scripts
    for section in cfg:
        if section in ["DEFAULT"]:
            continue

        section_data = cfg[section]
        section_dir = os.path.join(testdir, section)

        if "clean" in section_data:
            logging.info(f"Running {section} clean script (reason: prepare)")
            clean_scripts[section] = section_data["clean"]
            env = {"TESTENV_CLEAN_REASON": "prepare"}
            testenv.cmd.run(section_data["clean"], cwd=section_dir, env=env)

        if "prepare" in section_data:
            logging.info(f"Running {section} prepare script")
            testenv.cmd.run(section_data["prepare"], cwd=section_dir)


def clean():
    """Don't leave behind an empty testdir_topdir, e.g. if testenv.py aborted
    during build of components."""
    # Show log dir path/link if it isn't empty
    if os.listdir(testdir_topdir):
        msg = "Logs saved to:"

        url = os.environ.get("BUILD_URL")  # Jenkins sets this
        if url:
            # Add a space at the end, so jenkins can transform this into a link
            # without adding the color reset escape code to it
            msg += f" {url}artifact/logs/ "
        else:
            msg += f" {testdir_topdir}"
            if not testenv.args.log_dir:
                msg += " (symlink: /tmp/logs)"
        logging.info(msg)
        return

    logging.debug("Removing empty log dir")
    testenv.cmd.run(["rm", "-d", testdir_topdir], no_podman=True)

    # Remove broken symlink
    if not testenv.args.log_dir and os.path.lexists("/tmp/logs") and not os.path.exists("/tmp/logs"):
        testenv.cmd.run(["rm", "/tmp/logs"], no_podman=True)


def clean_run_scripts(reason="crashed"):
    global clean_scripts

    if not clean_scripts:
        return
    elif testenv.args.podman and not testenv.podman.is_running():
        logging.debug("Skipping clean up scripts, podman container has already stopped")
    else:
        for section, script in clean_scripts.items():
            logging.info(f"Running {section} clean script (reason: {reason})")
            env = {"TESTENV_CLEAN_REASON": reason}
            testenv.cmd.run(script, cwd=os.path.join(testdir, section), env=env)

    clean_scripts = {}
