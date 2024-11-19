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


def check_fftranscode():
    cmd = [
        "grep",
        "-q",
        "fftranscode",
        os.path.join(
            testenv.testsuite.ttcn3_hacks_dir_src,
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

    if not testenv.args.podman:
        check_fftranscode()


def mount_sys_fs_bpf():
    for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
        if testenv.testenv_cfg.get_ebpf(cfg):
            testenv.cmd.run('mount | grep -q " on /sys/fs/bpf" || sudo mount none /sys/fs/bpf -t bpf', no_podman=True)
            return
