# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import logging
import os
import subprocess
import sys
import testenv
import testenv.cmd
import testenv.requirements
import testenv.daemons
import time
import shlex


upstream_image_url = "https://cloud.debian.org/images/cloud/bookworm/latest/debian-12-generic-amd64.qcow2"
cache_dir = os.environ.get(
    "TESTENV_QEMU_CACHE_DIR", os.path.join(os.path.expanduser("~/.cache"), "osmo-ttcn3-testenv/qemu")
)
ssh_key = os.path.join(cache_dir, "id_ed25519")
image_path_base = os.path.join(cache_dir, os.path.basename(upstream_image_url))
image_path_testenv = image_path_base.replace(".qcow2", "-testenv.qcow2")
qemu_process = None
ssh_port = 22222  # FIXME: don't hardcode


def create_image_with_backing(image_path, backing_path=image_path_testenv):
    testenv.cmd.run(["qemu-img", "create", "-f", "qcow2", "-b", backing_path, "-F", "qcow2", image_path])
    testenv.cmd.run(["qemu-img", "resize", image_path, "5G"])


def stop():
    global qemu_process

    if not qemu_process:
        return
    logging.info(f"Stopping qemu ({qemu_process.pid})")
    testenv.daemons.kill(qemu_process.pid)
    qemu_process = None


def ssh_prepare():
    # SSH refuses to use keys with wrong permissions
    if not os.path.exists(ssh_key):
        testenv.cmd.run(["install", "-Dm600",
                         os.path.join(testenv.data_dir, "qemu/id_ed25519"),
                         ssh_key])


def ssh_run(cmd, quiet=False, *args, **kwargs):

    if isinstance(cmd, str):
        user_cmd = cmd
    else:
        user_cmd = ""
        for word in cmd:
            user_cmd += f"{shlex.quote(word)} "
        user_cmd = user_cmd.rstrip()

    ssh_cmd = [
        "ssh",
        "-o", "NoHostAuthenticationForLocalhost=yes",
        "-o", "ConnectTimeout=1",
        "-i", ssh_key,
        "-p", f"{ssh_port}",
        "-tt",
        "root@127.0.0.1",
        user_cmd
    ]

    if quiet:
        ssh_cmd = ["sh", "-c", f"{' '.join(ssh_cmd)} >/dev/null 2>&1"]
        return subprocess.run(ssh_cmd, *args, **kwargs)

    return testenv.cmd.run(ssh_cmd, *args, **kwargs)


def scp_run(scp_args):
    cmd = [
        "scp",
        "-o", "NoHostAuthenticationForLocalhost=yes",
        "-o", "ConnectTimeout=1",
        "-i", ssh_key,
        "-P", f"{ssh_port}",
    ] + scp_args
    testenv.cmd.run(cmd)


def ssh_wait():
    quiet = not getattr(testenv.args, "qemu_verbose", False)

    logging.info("Waiting until VM is reachable via SSH...")
    for i in range(20):
        time.sleep(1)
        p = ssh_run("true", quiet=quiet)
        if p.returncode == 0:
            return
    raise testenv.NoTraceException(f"Could not reach the VM via ssh! Try --qemu-verbose.")


def start(image_path, qemu_extra_args=[]):
    global qemu_process

    atexit.register(stop)

    cmd = [
            "qemu-system-x86_64",
            "-machine",
            "pc",
            "-drive",
            f"file={image_path},format=qcow2,if=virtio",
            "-enable-kvm",
            "-m",
            "1024",
            "-display",
            "none",
            "-device",
            "virtio-net-pci,netdev=net",
            "-netdev",
            f"user,id=net,hostfwd=tcp:127.0.0.1:{ssh_port}-:22",
            "-smp",
            "2",
        ] + qemu_extra_args

    if getattr(testenv.args, "qemu_verbose", False):
        cmd += ["-serial", "stdio"]

    logging.info(f"Running qemu")
    logging.debug(f"+ {cmd}")
    qemu_process = subprocess.Popen(cmd)
    testenv.daemons.check_startup_failed(qemu_process, "qemu")
    ssh_prepare()
    ssh_wait()
