# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import shlex
import string
import subprocess
import sys
import testenv.cmd
import testenv.podman


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
                    grep = subprocess.run(["grep", "-q", f"^{dbg_pkg}$", dbg_pkgs_all], check=False)

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
            logging.error(f"Missing package= line in [{section}] of {cfg_name}!")
            logging.error("See _testenv/README.md for more information.")
            sys.exit(1)

    apt_install(packages)
