# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
from packaging.version import Version
import logging
import os.path
import shutil
import subprocess
import sys
import testenv
import testenv.cmd
import testenv.testsuite


def check_programs():
    programs = [
        "git",
    ]

    if testenv.args.podman:
        programs += [
            "buildah",
            "podman",
            "rsync",
        ]
    else:
        programs += [
            "autoconf",
            "automake",
            "brctl",
            "ccache",
            "dumpcap",
            "g++",
            "gcc",
            "make",
            "pkg-config",
            "setcap",
            "ttcn3_compiler",
            "wget",
        ]

        if testenv.args.kernel:
            programs += [
                "busybox",
                "cpio",
                "gzip",
                "lddtree",
                "qemu-system-x86_64",
            ]
        if not testenv.args.binary_repo:
            programs += [
                "rsync",
            ]

    abort = False
    for program in programs:
        if not shutil.which(program):
            if os.path.exists(os.path.join("/usr/sbin", program)):
                # Debian: some programs such as setcap are in /usr/sbin, which
                # is not in PATH unless using sudo. Therefore "shutil.which()"
                # won't find it.
                continue

            logging.error(f"Missing program: {program}")

            if program == "ttcn3_compiler":
                logging.error("  Install eclipse-titan, e.g. from osmocom:latest:")
                logging.error("  https://osmocom.org/projects/cellular-infrastructure/wiki/Latest_Builds")
            abort = True

    if abort:
        sys.exit(1)


def check_ttcn3_compiler_version():
    if testenv.args.podman:
        return

    ttcn3_min_version = "9.0.0"

    v = subprocess.run(["ttcn3_compiler", "-v"], capture_output=True, text=True)
    current = None
    for line in v.stderr.split("\n"):
        if not line.startswith("Version: "):
            continue

        current = line.split(":", 1)[1].strip()
        if Version(current) >= Version(ttcn3_min_version):
            return

        logging.error(f"Found ttcn3_compiler version {current}, but {ttcn3_min_version} or higher is required.")

    if not current:
        logging.error("Could not parse ttcn3_compiler version.")
    logging.error(f"Please install at least version {ttcn3_min_version} and try again.")
    sys.exit(1)


def check_fftranscode():
    cmd = [
        "grep",
        "-q",
        "fftranscode",
        os.path.join(
            testenv.testsuite.ttcn3_hacks_dir,
            testenv.args.testsuite,
            "regen_makefile.sh",
        ),
    ]
    if testenv.cmd.run(cmd, check=False).returncode == 1:
        return

    cmd = ["pkg-config", "--modversion", "libfftranscode"]
    if testenv.cmd.run(cmd, check=False).returncode == 0:
        return

    logging.error("Missing library: libfftranscode")
    logging.error(
        "  https://osmocom.org/projects/cellular-infrastructure/wiki/Titan_TTCN3_Testsuites#Proprietary-APERlt-gtBER-transcoding-library-for-Iu-tests"
    )
    logging.error("  Consider installing it from here:")
    logging.error("  https://ftp.osmocom.org/binaries/libfftranscode/")
    sys.exit(1)


def check():
    check_programs()
    check_ttcn3_compiler_version()

    if not testenv.args.podman:
        check_fftranscode()
