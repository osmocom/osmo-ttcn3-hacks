# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import multiprocessing
import os
import sys
import testenv.cmd
import testenv.podman

git_dir = None
bb_dir = None
trxcon_dir = None
sccp_dir = None
jobs = None


def init():
    global git_dir
    global bb_dir
    global trxcon_dir
    global sccp_dir
    global jobs

    git_dir = os.path.join(testenv.args.cache, "git")
    bb_dir = os.path.join(git_dir, "osmocom-bb")
    trxcon_dir = os.path.join(bb_dir, "src/host/trxcon")
    sccp_dir = os.path.join(git_dir, "libosmo-sigtran")
    jobs = multiprocessing.cpu_count() + 1

    os.makedirs(git_dir, exist_ok=True)


def apt_install(pkgs):
    if not pkgs:
        return

    # Remove duplicates
    pkgs = list(set(pkgs))

    logging.info(f"Installing packages: {', '.join(pkgs)}")
    testenv.cmd.run(["apt-get", "-q", "install", "-y", "--no-install-recommends"] + pkgs)


def clone_osmocom_bb():
    if os.path.exists(bb_dir):
        logging.debug("osmocom-bb: already cloned")
        return

    testenv.cmd.run(
        [
            "git",
            "-C",
            git_dir,
            "clone",
            "--depth",
            "1",
            "https://gerrit.osmocom.org/osmocom-bb",
        ]
    )


def clone_libosmo_sigtran():
    if os.path.exists(sccp_dir):
        logging.debug("libosmo-sigtran: already cloned")
        return

    testenv.cmd.run(
        [
            "git",
            "-C",
            git_dir,
            "clone",
            "--depth",
            "1",
            "https://gerrit.osmocom.org/libosmo-sigtran",
        ]
    )


def from_source_trxcon():
    trxcon_in_srcdir = os.path.join(trxcon_dir, "src/trxcon")

    if not os.path.exists(trxcon_in_srcdir):
        clone_osmocom_bb()
        apt_install(["libosmocore-dev"])
        logging.info("Building trxcon")
        testenv.cmd.run(["autoreconf", "-fi"], cwd=trxcon_dir)
        testenv.cmd.run(["./configure"], cwd=trxcon_dir)
        testenv.cmd.run(["make", "-j", f"{jobs}"], cwd=trxcon_dir)

    testenv.cmd.run(["ln", "-s", trxcon_in_srcdir, "/usr/local/bin/trxcon"])


def from_source_sccp_demo_user():
    sccp_demo_user_path = os.path.join(sccp_dir, "examples/sccp_demo_user")

    # Install libraries even if not building sccp_demo_user, because it gets
    # linked dynamically against them.
    apt_install(
        [
            "libosmo-netif-dev",
            "libosmocore-dev",
        ]
    )

    if not os.path.exists(sccp_demo_user_path):
        clone_libosmo_sigtran()
        logging.info("Building sccp_demo_user")
        testenv.cmd.run(["autoreconf", "-fi"], cwd=sccp_dir)
        testenv.cmd.run(["./configure"], cwd=sccp_dir)
        testenv.cmd.run(
            ["make", "-j", f"{jobs}", "libosmo-sigtran.la"],
            cwd=os.path.join(sccp_dir, "src"),
        )
        testenv.cmd.run(
            ["make", "-j", f"{jobs}", "sccp_demo_user"],
            cwd=os.path.join(sccp_dir, "examples"),
        )

    testenv.cmd.run(["ln", "-s", sccp_demo_user_path, "/usr/local/bin/sccp_demo_user"])


def from_source(cfg, cfg_name, section):
    program = cfg[section]["program"]
    if program == "trxcon":
        return from_source_trxcon()
    if program == "run_fake_trx.sh":
        return clone_osmocom_bb()
    if program == "run_sccp_demo_user.sh":
        return from_source_sccp_demo_user()

    logging.error(f"Can't install {section}! Fix this by either:")
    logging.error(f"* Adding package= to [{section}] in {cfg_name}")
    logging.error("  (if it can be installed from binary packages)")
    logging.error("* Editing from_source() in testenv/podman_install.py")
    sys.exit(1)


def packages(cfg, cfg_name):
    packages = []

    for section in cfg:
        if section in ["DEFAULT", "testsuite"]:
            continue
        section_data = cfg[section]
        if "package" in section_data:
            if section_data["package"] == "no":
                continue
            packages += section_data["package"].split(" ")
        else:
            from_source(cfg, cfg_name, section)

    apt_install(packages)
