# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import glob
import logging
import multiprocessing
import os
import os.path
import shlex
import shutil
import subprocess
import testenv
import testenv.cmd
import time

ttcn3_hacks_dir = None
ttcn3_hacks_dir_src = os.path.realpath(f"{__file__}/../../..")
testsuite_proc = None


def update_deps():
    deps_marker = os.path.join(testenv.args.cache, "ttcn3-deps-updated")
    if os.path.exists(deps_marker):
        return

    logging.info("Updating osmo-ttcn3-hacks/deps")
    deps_dir = os.path.join(ttcn3_hacks_dir_src, "deps")
    testenv.cmd.run(["make", "-C", deps_dir])
    testenv.cmd.run(["touch", deps_marker])


def copy_ttcn3_hacks_dir():
    """Copy source files of osmo-ttcn3-hacks.git to the cache dir, so we don't
    mix binary objects from host and inside podman that are very likely to
    be incompatible"""
    global ttcn3_hacks_dir

    ttcn3_hacks_dir = os.path.join(testenv.args.cache, "podman", "osmo-ttcn3-hacks")

    logging.info(f"Copying osmo-ttcn3-hacks sources to: {ttcn3_hacks_dir}")

    # Rsync can't directly parse the .gitignore with ! rules, so create a list
    # of files to be copied with git
    copy_list = os.path.join(testenv.args.cache, "podman", "ttcn3-copy-list")
    testenv.cmd.run(
        f"git ls-files -o -c --exclude-standard > {shlex.quote(copy_list)}",
        cwd=ttcn3_hacks_dir_src,
        no_podman=True,
    )

    # Copy source files, excluding binary objects
    testenv.cmd.run(
        [
            "rsync",
            "--links",
            "--recursive",
            f"--files-from={copy_list}",
            f"{ttcn3_hacks_dir_src}/",
            f"{ttcn3_hacks_dir}/",
        ],
        no_podman=True,
    )

    # The "deps" dir is in gitignore, copy it separately
    testenv.cmd.run(
        [
            "rsync",
            "--links",
            "--recursive",
            "--exclude",
            "/.git",
            f"{ttcn3_hacks_dir_src}/deps/",
            f"{ttcn3_hacks_dir}/deps/",
        ],
        no_podman=True,
    )


def prepare_testsuite_dir():
    testsuite_dir = f"{ttcn3_hacks_dir}/{testenv.args.testsuite}"
    logging.info(f"Generating links and Makefile for {testenv.args.testsuite}")
    testenv.cmd.run(["./gen_links.sh"], cwd=testsuite_dir)
    testenv.cmd.run("USE_CCACHE=1 ./regen_makefile.sh", cwd=testsuite_dir)


def init():
    global ttcn3_hacks_dir

    atexit.register(stop)

    update_deps()

    if testenv.args.podman:
        copy_ttcn3_hacks_dir()
    else:
        ttcn3_hacks_dir = ttcn3_hacks_dir_src

    prepare_testsuite_dir()


def build():
    logging.info("Building testsuite")
    testsuite_dir = f"{ttcn3_hacks_dir}/{testenv.args.testsuite}"
    testenv.cmd.run(["make", "compile"], cwd=testsuite_dir)

    jobs = multiprocessing.cpu_count() + 1
    testenv.cmd.run(["make", "-j", f"{jobs}"], cwd=testsuite_dir)


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


def cat_junit_logs():
    tool = "cat"

    if testenv.args.podman or shutil.which("source-highlight"):
        colors = os.environ.get("TESTENV_SOURCE_HIGHLIGHT_COLORS", "esc256")
        tool = f"source-highlight -f {shlex.quote(colors)} -s xml -i"

    pattern = os.path.join(testenv.testdir.testdir, "testsuite", "junit-*.log")
    for path in glob.glob(pattern):
        cmd = f"echo && {tool} {shlex.quote(path)} && echo"
        logging.info(f"Showing {os.path.basename(path)}")
        testenv.cmd.run(cmd)


def run(cfg):
    global testsuite_proc

    section_data = cfg["testsuite"]

    cwd = os.path.join(testenv.testdir.testdir, "testsuite")
    start_testsuite = os.path.join(ttcn3_hacks_dir, "start-testsuite.sh")
    suite = os.path.join(ttcn3_hacks_dir, testenv.args.testsuite, section_data["program"])

    env = {
        "TTCN3_PCAP_PATH": os.path.join(testenv.testdir.testdir, "testsuite"),
    }
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
            logging.debug("Testsuite is done")
            stop()
            break

        for daemon_name, daemon_proc in testenv.daemons.daemons.items():
            if not is_running(daemon_proc.pid):
                raise testenv.NoTraceException(f"{daemon_name} crashed!")

    merge_log_files(cfg)
    format_log_files(cfg)


def run_prepare_script(cfg):
    section_data = cfg["testsuite"]
    if "prepare" not in section_data:
        return

    logging.info("Running testsuite prepare script")
    testenv.cmd.run(section_data["prepare"])


def stop():
    global testsuite_proc

    if testsuite_proc:
        logging.info(f"Stopping testsuite ({testsuite_proc.pid})")
        testenv.daemons.kill(testsuite_proc.pid)
        testsuite_proc = None
