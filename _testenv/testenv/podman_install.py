# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import multiprocessing
import os
import packaging.version
import re
import shlex
import string
import subprocess
import sys
import testenv.cmd
import testenv.podman

git_dir = None
jobs = None


def init():
    global git_dir
    global jobs

    # Make the git dir we clone into specific to the repository we build
    # against. Replace ":" because the libtool scripts fail to escape it when
    # setting LD_LIBRARY_PATH, leading to "cannot open shared object file"
    # errors.
    git_dir = os.path.join(testenv.args.cache, "git", f"build_against_{testenv.args.binary_repo}".replace(":", "_"))

    if testenv.args.jobs:
        jobs = testenv.args.jobs
    else:
        jobs = multiprocessing.cpu_count()

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


def show_commit(project, cwd):
    cmd = ["git", "-P", "-C", cwd, "-c", "color.ui=always", "log", "-1", "--oneline"]
    logging.info(f"{project}: showing current commit")
    testenv.cmd.run(cmd, no_podman=True)


def clone_project(project):
    git_dir_project = os.path.join(git_dir, project)
    if os.path.exists(git_dir_project):
        logging.debug(f"{project}: already cloned")
        show_commit(project, git_dir_project)
        return

    branch = "master"
    url = f"https://gerrit.osmocom.org/{project}"
    if testenv.args.binary_repo.endswith(":latest"):
        ls_remote = testenv.cmd.run(["git", "ls-remote", "--tags", url], capture_output=True, text=True, no_podman=True)
        tags = []
        pattern = re.compile("^\d+\.\d+\.\d+$")
        for line in ls_remote.stdout.split("\n"):
            if "refs/tags/" in line:
                tag = line.split("refs/tags/")[1].split("^")[0]
                if pattern.match(tag):
                    tags += [tag]
        tags.sort(key=packaging.version.Version, reverse=True)
        branch = tags[0]

    logging.info(f"{project}: cloning {branch}")
    testenv.cmd.run(
        [
            "git",
            "-c",
            "advice.detachedHead=false",
            "-C",
            git_dir,
            "clone",
            "--depth",
            "1",
            "--branch",
            branch,
            url,
        ],
        no_podman=True,
    )
    show_commit(project, git_dir_project)


def from_source_sccp_demo_user():
    sccp_dir = os.path.join(git_dir, "libosmo-sigtran")
    sccp_demo_user_path = os.path.join(sccp_dir, "examples/sccp_demo_user")

    # Install libraries even if not building sccp_demo_user, because it gets
    # linked dynamically against them.
    apt_install(
        [
            "libosmo-netif-dev",
            "libosmocore-dev",
        ]
    )

    clone_project("libosmo-sigtran")
    if not os.path.exists(sccp_demo_user_path):
        logging.info("Building sccp_demo_user")
        testenv.cmd.run(["autoreconf", "-fi"], cwd=sccp_dir)

        configure_cmd = ["./configure"]
        if testenv.args.binary_repo.endswith(":asan"):
            configure_cmd += ["--enable-sanitize"]
        testenv.cmd.run(configure_cmd, cwd=sccp_dir)

        testenv.cmd.run(
            ["make", "-j", f"{jobs}", "libosmo-sigtran.la"],
            cwd=os.path.join(sccp_dir, "src"),
        )
        testenv.cmd.run(
            ["make", "-j", f"{jobs}", "sccp_demo_user"],
            cwd=os.path.join(sccp_dir, "examples"),
        )

    testenv.cmd.run(["ln", "-s", sccp_demo_user_path, "/usr/local/bin/sccp_demo_user"])


def from_source_osmo_ns_dummy():
    libosmocore_dir = os.path.join(git_dir, "libosmocore")
    osmo_ns_dummy_path = os.path.join(libosmocore_dir, "utils/osmo-ns-dummy")

    clone_project("libosmocore")
    if not os.path.exists(osmo_ns_dummy_path):
        logging.info("Building osmo-ns-dummy")
        testenv.cmd.run(["autoreconf", "-fi"], cwd=libosmocore_dir)

        configure_cmd = ["./configure"]
        if testenv.args.binary_repo.endswith(":asan"):
            configure_cmd += ["--enable-sanitize"]
        testenv.cmd.run(configure_cmd, cwd=libosmocore_dir)

        testenv.cmd.run(
            [os.path.join(testenv.data_dir, "scripts/build_osmo_ns_dummy.sh"), str(jobs)], cwd=libosmocore_dir
        )

    testenv.cmd.run(["ln", "-s", osmo_ns_dummy_path, "/usr/local/bin/osmo-ns-dummy"])


def from_source(cfg, cfg_name, section):
    program = cfg[section]["program"].split(" ", 1)[0]
    if program == "run_sccp_demo_user.sh":
        return from_source_sccp_demo_user()
    if program == "run_osmo_ns_dummy.sh":
        return from_source_osmo_ns_dummy()

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
