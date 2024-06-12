import logging
import os
import shlex
import sys
import testenv
import testenv.cmd

make_dir = None
init_done = False


def init():
    global init_done
    global make_dir

    if init_done:
        return

    extra_opts = []
    if testenv.args.podman:
        make_dir = os.path.join(testenv.args.cache, "podman", "make")
        extra_opts = [
            "ccache.opts",
            "no_uring.opts",
            "--install-prefix", os.path.join(testenv.args.cache, "podman/usr"),
        ]
    else:
        make_dir = os.path.join(testenv.args.cache, "host", "make")
        extra_opts = [
            "--install-prefix", os.path.join(testenv.args.cache, "host/usr"),
        ]

    osmo_dev_dir = os.path.join(testenv.top_dir, "osmo-dev")

    if not os.path.exists(osmo_dev_dir):
        logging.info(f"osmo-dev: cloning to: {osmo_dev_dir}")
        testenv.cmd.run(["git", "clone", "https://gerrit.osmocom.org/osmo-dev"],
                         cwd=testenv.top_dir)

    cmd = [
        "./gen_makefile.py",
        "-s", testenv.top_dir,
        "-m", make_dir,
        "--no-ldconfig",
        "no_dahdi.opts",
        "no_doxygen.opts",
        "no_systemd.opts",
        "werror.opts",
        os.path.join(testenv.data_dir, "osmo-dev/osmo-bts-trx.opts"),
    ] + extra_opts

    testenv.cmd.run(cmd, cwd=osmo_dev_dir)
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

        if "make" in section_data and section_data["make"] != "no":
            targets += [section_data["make"]]

    if targets:
        logging.info(f"Building test components")
        testenv.cmd.run(["make"] + targets, cwd=make_dir)
