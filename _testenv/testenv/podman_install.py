# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import multiprocessing
import os
import shlex
import string
import subprocess
import sys
import testenv.cmd
import testenv.podman

git_dir = None
sccp_dir = None
jobs = None


def init():
    global git_dir
    global sccp_dir
    global jobs

    git_dir = os.path.join(testenv.args.cache, "git")
    sccp_dir = os.path.join(git_dir, "libosmo-sigtran")
    jobs = multiprocessing.cpu_count() + 1

    os.makedirs(git_dir, exist_ok=True)


def get_dbg_pkgs(dep):
    ret = [f"{dep}-dbg", f"{dep}-dbgsym"]

    # Get from e.g. libosmocore22 to libosmocore-dbg
    dep_nodigits = dep.rstrip(string.digits)
    if dep_nodigits != dep:
        ret += [f"{dep_nodigits}-dbg", f"{dep_nodigits}-dbgsym"]

    return ret


def apt_get_dbg_pkgs(pkgs):
    dbg_pkgs_all = os.path.join(testenv.args.cache, "podman", "dbg_pkgs_all")
    dbg_pkgs = {}

    testenv.cmd.run(f"apt-cache pkgnames | grep -- -dbg > {shlex.quote(dbg_pkgs_all)}")

    for pkg in pkgs:
        # Iterate over apt-rdepends, example output:
        # osmo-mgw
        #   Depends: libc6 (>= 2.34)
        #   Depends: libosmoabis13
        rdeps = testenv.cmd.run(["apt-rdepends", pkg], stdout=subprocess.PIPE)
        for line in rdeps.stdout.decode("utf-8").split("\n"):
            if line.startswith("  "):
                continue
            dep = line.rstrip().split(" ", 1)[0]

            if dep not in dbg_pkgs:
                for dbg_pkg in get_dbg_pkgs(dep):
                    # Use subprocess.run so we don't get lots of log messages.
                    # Also we don't need to run grep through podman.
                    grep = subprocess.run(["grep", "-q", f"^{dbg_pkg}$", dbg_pkgs_all])

                    if grep.returncode == 0:
                        dbg_pkgs[dep] = dbg_pkg
                        break

                if dep not in dbg_pkgs:
                    dbg_pkgs[dep] = None

            if dbg_pkgs[dep]:
                logging.debug(f"{pkg} -> {dep}: installing {dbg_pkgs[dep]}")

    ret = []
    for dep, dbg in dbg_pkgs.items():
        if dbg:
            ret += [dbg]

    return ret


def apt_install(pkgs):
    if not pkgs:
        return

    # Remove duplicates
    pkgs = list(set(pkgs))

    pkgs += apt_get_dbg_pkgs(pkgs)

    logging.info(f"Installing packages: {', '.join(pkgs)}")
    testenv.cmd.run(["apt-get", "-q", "install", "-y", "--no-install-recommends"] + pkgs)


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
    program = cfg[section]["program"].split(" ", 1)[0]
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
