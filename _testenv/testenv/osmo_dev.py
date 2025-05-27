# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import shlex
import sys
import testenv
import testenv.cmd

init_done = False


def get_osmo_dev_dir():
    # Users may have used osmo-dev to clone osmo-ttcn3-hacks:
    # osmo-dev/src/osmo-ttcn3-hacks
    alt_path = os.path.realpath(os.path.join(testenv.src_dir, "../"))
    if os.path.exists(os.path.join(alt_path, "gen_makefile.py")):
        return alt_path

    # Assume osmo-dev is next to osmo-ttcn3-hacks:
    # src_dir
    # ├── osmo-dev
    # └── osmo-ttcn3-hacks
    return os.path.join(testenv.src_dir, "osmo-dev")


def init_clone():
    osmo_dev_dir = get_osmo_dev_dir()

    if os.path.exists(osmo_dev_dir):
        logging.debug(f"osmo-dev found, nothing to do: {osmo_dev_dir}")
        return

    testenv.cmd.run(["git", "clone", "https://gerrit.osmocom.org/osmo-dev"], cwd=testenv.src_dir)


def check_init_needed():
    osmo_dev_dir = get_osmo_dev_dir()

    if os.path.exists(osmo_dev_dir):
        logging.debug(f"osmo-dev dir: {osmo_dev_dir}")
        return

    logging.error("Missing osmo-dev for building test components from source.")
    logging.error("Run 'testenv.py init osmo-dev' first.")
    logging.error("")
    logging.error("osmo-dev and other Osmocom repositories (if they don't already exist) will be cloned to:")
    logging.error(testenv.src_dir)
    logging.error("")
    logging.error("Set the environment variable TESTENV_SRC_DIR to use a different path.")
    sys.exit(1)


def init():
    global init_done

    if init_done:
        return

    extra_opts = []
    if testenv.args.jobs:
        extra_opts += [f"-j{testenv.args.jobs}"]

    # Bump testenv.cmd.make_dir_version when making incompatible changes
    cmd = [
        "./gen_makefile.py",
        "--build-debug",
        "--no-make-check",
        "--install-prefix",
        testenv.cmd.install_dir,
        "--make-dir",
        testenv.cmd.make_dir,
        "--no-ldconfig",
        "--src-dir",
        testenv.src_dir,
        "default.opts",
        "ccache.opts",
        "gtp_linux.opts",
        "hnbgw_with_nftables.opts",
        "hnbgw_with_pfcp.opts",
        "iu.opts",
        "no_dahdi.opts",
        "no_doxygen.opts",
        "no_man_pages.opts",
        "no_systemd.opts",
        "werror.opts",
        os.path.join(testenv.data_dir, "osmo-dev/testenv.opts"),
        "--autoreconf-in-src-copy",
    ] + extra_opts

    cwd = get_osmo_dev_dir()
    if testenv.cmd.run(cmd, cwd=cwd, check=False).returncode:
        logging.critical("gen_makefile.py from osmo-dev failed!")
        logging.critical("Your osmo-dev.git clone might be outdated, try:")
        logging.critical(f"$ git -C {shlex.quote(cwd)} pull")
        sys.exit(1)
    init_done = True


def make(cfg, limit_section=None):
    targets = []

    for section in cfg:
        section_data = cfg[section]
        if section == "testsuite":
            # Gets built with testenv.testsuite.build()
            continue
        if limit_section and limit_section != section:
            # When called from testenv.podman.install_packages as fallback to
            # not having a package available, then we only want to run make
            # for the target of one specific config section
            continue

        if "make" in section_data and section_data["make"] != "no" and section_data["make"] not in targets:
            targets += [section_data["make"]]

    if not targets:
        logging.debug("No osmo-dev make targets found in testenv.cfg")
        return

    makefile_path = os.path.join(testenv.cmd.make_dir, "Makefile")
    with open(makefile_path) as f:
        makefile = f.read()

    for target in targets:
        if f"\n{target}:" in makefile:
            continue
        logging.error(f"Could not find make target: {target}")
        logging.error("Add it to osmo-dev by adjusting:")
        logging.error("* all.deps")
        logging.error("* all.buildsystems (if buildsystem != autotools)")
        logging.error("* all.urls (if the project is not on gerrit.osmocom.org)")
        logging.error("Location of your osmo-dev.git clone:")
        logging.error(os.path.join(testenv.src_dir, "osmo-dev"))
        sys.exit(1)

    logging.info("Building test components")
    testenv.cmd.run(["make"] + targets, cwd=testenv.cmd.make_dir)
