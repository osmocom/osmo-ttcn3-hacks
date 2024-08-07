# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import os
import subprocess
import testenv
import testenv.cmd
import testenv.requirements
import testenv.qemu
import testenv.daemons
import logging
import time
import sys
import atexit

imds_httpd = None
init_done_marker = os.path.join(testenv.qemu.cache_dir, "init_is_done")


def stop_imds_httpd():
    global imds_httpd

    if not imds_httpd:
        return

    logging.info(f"Stopping imds httpd ({imds_httpd.pid})")
    testenv.daemons.kill(imds_httpd.pid)
    imds_httpd = None


def start_imds_httpd(port):
    global imds_httpd

    atexit.register(stop_imds_httpd)
    imds_dir = os.path.join(testenv.data_dir, "qemu/imds")
    cmd = ["python3", "-m", "http.server", "-d", imds_dir, "-b", "127.0.0.1", f"{port}"]

    logging.info(f"Running imds httpd")
    logging.debug(f"+ {cmd}")
    imds_httpd = subprocess.Popen(cmd)
    testenv.daemons.check_startup_failed(imds_httpd, "imds httpd")


def init():
    if os.path.exists(init_done_marker):
        logging.debug(f"QEMU VM was already created")
        if testenv.args.force:
            logging.debug("Creating a new VM since --force was used")
            testenv.cmd.run(["rm", init_done_marker])
        else:
            return

    os.makedirs(testenv.qemu.cache_dir, exist_ok=True)

    if not os.path.exists(testenv.qemu.image_path_base):
        logging.info(f"Downloading {testenv.qemu.upstream_image_url}")
        testenv.cmd.run(["wget", testenv.qemu.upstream_image_url, "-O", testenv.qemu.image_path_base])

    testenv.qemu.create_image_with_backing(testenv.qemu.image_path_testenv, testenv.qemu.image_path_base)

    # Run the VM and configure it via imds
    imds_port = 1337  # FIXME
    start_imds_httpd(imds_port)
    testenv.qemu.start(testenv.qemu.image_path_testenv,
                       ["-smbios", f"type=1,serial=ds=nocloud-net;s=http://10.0.2.2:{imds_port}/"])
    stop_imds_httpd()

    # Copy files to the VM
    for src_file in ["obs.key", "setup.sh"]:
        src_path = os.path.join(testenv.data_dir, "virt", src_file)
        testenv.qemu.scp_run([src_path, f"root@127.0.0.1:/"])

    # Run setup script
    testenv.qemu.ssh_run(["sh", "-ex", "/setup.sh"])

    # Set a marker file, the image has been built
    testenv.cmd.run(["touch", init_done_marker])
