# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import copy
import glob
import logging
import os
import os.path
import shlex
import shutil
import subprocess
import testenv
import testenv.cmd
import time

ttcn3_hacks_dir = os.path.realpath(f"{__file__}/../../..")
builddir_env = {}
testsuite_proc = None


def update_deps():
    logging.info("Updating osmo-ttcn3-hacks/deps")
    testenv.cmd.run(["make", "deps"], cwd=ttcn3_hacks_dir)


def prepare_testsuite_dir():
    testsuite_dir = f"{ttcn3_hacks_dir}/{testenv.args.testsuite}"
    logging.info(f"Generating links and Makefile for {testenv.args.testsuite}")
    testenv.cmd.run(["./gen_links.sh"], cwd=testsuite_dir, env=builddir_env)
    testenv.cmd.run("USE_CCACHE=1 ./regen_makefile.sh", cwd=testsuite_dir, env=builddir_env)


def init():
    global builddir_env

    atexit.register(stop)
    update_deps()

    if testenv.args.podman:
        builddir = os.path.join(testenv.args.cache, "podman", "ttcn3")
        builddir_env = {"BUILDDIR": builddir}

    prepare_testsuite_dir()


def build():
    logging.info("Building testsuite")

    env = copy.copy(builddir_env)
    if testenv.args.jobs:
        env["PARALLEL_MAKE"] = f"-j{testenv.args.jobs}"

    testenv.cmd.run(["make", testenv.args.testsuite], cwd=ttcn3_hacks_dir, env=env)


def is_running(pid):
    # Check if a process is still running, or if it is dead / a zombie. We
    # can't just use proc.poll() because this gets called from another thread.
    cmdline = f"/proc/{pid}/cmdline"
    if not os.path.exists(cmdline):
        return False

    # The cmdline file is empty if it is a zombie
    with open(cmdline) as f:
        return f.read() != ""


def merge_log_files(cfg):
    section_data = cfg["testsuite"]
    cwd = os.path.join(testenv.testdir.testdir, "testsuite")

    logging.info("Merging log files")
    log_merge = os.path.join(ttcn3_hacks_dir, "log_merge.sh")
    # stdout of this script is very verbose, making it harder to see the output
    # that matters (tests failed or not), so redirect it to /dev/null
    cmd = f"{shlex.quote(log_merge)} {shlex.quote(section_data['program'])} --rm >/dev/null"
    testenv.cmd.run(cmd, cwd=cwd)


def format_log_files(cfg):
    cwd = os.path.join(testenv.testdir.testdir, "testsuite")

    logging.info("Formatting log files")
    cmd = os.path.join(testenv.data_dir, "scripts/log_format.sh")
    testenv.cmd.run(cmd, cwd=cwd)


def get_junit_logs(topdir):
    pattern = os.path.join(topdir, "**", "junit-*.log")
    return glob.glob(pattern, recursive=True)


def cat_junit_logs():
    tool = "cat"

    if testenv.args.podman or shutil.which("source-highlight"):
        colors = os.environ.get("TESTENV_SOURCE_HIGHLIGHT_COLORS", "esc256")
        tool = f"source-highlight -f {shlex.quote(colors)} -s xml -i"

    for path in get_junit_logs(testenv.testdir.testdir_topdir):
        cmd = f"echo && {tool} {shlex.quote(path)} && echo"
        logging.info(f"Showing {os.path.relpath(path, testenv.testdir.testdir_topdir)}")
        testenv.cmd.run(cmd)


def check_testsuite_successful(loop_count=None):
    topdir = testenv.testdir.testdir_topdir
    if loop_count is not None:
        topdir = os.path.join(topdir, f"loop-{loop_count}")

    ret = True

    for path in get_junit_logs(topdir):
        for match_str in ["failures='0'", "errors='0'"]:
            cmd = ["grep", "-q", match_str, path]
            if testenv.cmd.run(cmd, check=False).returncode:
                ret = False
                break
        if not ret:
            break

    return ret


def run(cfg):
    global testsuite_proc

    section_data = cfg["testsuite"]

    cwd = os.path.join(testenv.testdir.testdir, "testsuite")
    start_testsuite = os.path.join(ttcn3_hacks_dir, "start-testsuite.sh")
    suite = os.path.join(ttcn3_hacks_dir, testenv.args.testsuite, section_data["program"])
    suite = os.path.relpath(suite, ttcn3_hacks_dir)

    env = copy.copy(builddir_env)
    env["TTCN3_PCAP_PATH"] = os.path.join(testenv.testdir.testdir, "testsuite")

    # Let ttcn3-tcpdump-stop.sh retrieve talloc reports
    host, port = testenv.testenv_cfg.get_vty_host_port(cfg)
    if port:
        env["OSMO_SUT_HOST"] = host
        env["OSMO_SUT_PORT"] = port

    env = testenv.cmd.generate_env(env, testenv.args.podman)

    cmd = [start_testsuite, suite, section_data["config"]]

    test_arg = testenv.args.test
    if test_arg:
        if "." in test_arg:
            cmd += [test_arg]
        else:
            cmd += [f"{section_data['program']}.{test_arg}"]

    logging.info("Running testsuite")

    if testenv.podman.is_running():
        testsuite_proc = testenv.podman.exec_cmd_background(cmd, cwd=cwd, env=env)
    else:
        logging.debug(f"+ {cmd}")
        testsuite_proc = subprocess.Popen(cmd, cwd=cwd, env=env)

    # Ensure all daemons run until the testsuite stops
    while True:
        time.sleep(1)

        if not is_running(testsuite_proc.pid):
            if testenv.args.podman and not testenv.podman.is_running():
                raise testenv.NoTraceException("podman container crashed!")
            logging.debug("Testsuite is done")
            stop()
            break

        testenv.daemons.check_if_crashed()

    merge_log_files(cfg)
    format_log_files(cfg)

    if testenv.args.bisect and not check_testsuite_successful():
        raise testenv.NoTraceException("Testsuite failed!")


def get_current_test():
    path = os.path.join(testenv.testdir.testdir, "testsuite/.current_test")
    try:
        with open(path, "r") as h:
            return h.readline().rstrip()
    except:  # noqa
        # File may not exist, e.g. if test was stopped
        return None


def wait_until_test_stopped():
    path = os.path.join(testenv.testdir.testdir, "testsuite/.current_test")

    logging.debug("Waiting until test has stopped...")

    for i in range(0, 1200):
        time.sleep(0.1)
        if not os.path.exists(path):
            return

    raise testenv.NoTraceError("Timeout in wait_until_test_stopped()")


def stop():
    global testsuite_proc

    if testsuite_proc:
        logging.info(f"Stopping testsuite ({testsuite_proc.pid})")
        testenv.daemons.kill(testsuite_proc.pid)
        testsuite_proc = None
