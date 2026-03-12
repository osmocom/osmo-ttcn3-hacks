# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os.path
import shutil
import sys
import testenv
import testenv.cmd
import testenv.testsuite


def check_programs():
    programs = [
        "file",
        "git",
    ]

    if testenv.args.podman:
        programs += [
            "buildah",
            "podman",
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
            "rsync",
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
                logging.error("  https://osmocom.org/projects/cellular-infrastructure/wiki/Binary_Packages")
            abort = True

    if abort:
        sys.exit(1)


def check():
    check_programs()
