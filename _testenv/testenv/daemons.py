# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import logging
import os
import os.path
import shlex
import subprocess
import testenv
import testenv.coredump
import testenv.testdir
import time
import sys

daemons = {}
run_shell_on_stop = False


def init():
    global run_shell_on_stop

    atexit.register(testenv.coredump.get_backtrace)

    if not testenv.args.podman:
        atexit.register(stop)
        if testenv.args.shell:
            run_shell_on_stop = True


def start(cfg):
    global daemons

    for section in cfg:
        if section in ["testenv", "DEFAULT"]:
            continue

        section_data = cfg[section]
        if "program" not in section_data:
            continue
        if section == "testsuite":
            # Runs in the foreground with testenv.testsuite.run()
            continue

        program = section_data["program"]
        logging.info(f"Running {section}")

        cwd = os.path.join(testenv.testdir.testdir, section)
        os.makedirs(cwd, exist_ok=True)

        log = os.path.join(testenv.testdir.testdir, section, f"{section}.log")

        if testenv.args.tee:
            pipe = f"2>&1 | tee {shlex.quote(log)}"
        else:
            pipe = f">{shlex.quote(log)} 2>&1"
        cmd = ["sh", "-c", f"{program} {pipe}"]

        env = {}
        if testenv.args.io_uring:
            env["LIBOSMO_IO_BACKEND"] = "IO_URING"

        if testenv.args.podman:
            daemons[section] = testenv.podman.exec_cmd_background(cmd, cwd=cwd, env=env)
        else:
            logging.debug(f"+ {cmd}")
            daemons[section] = subprocess.Popen(cmd, cwd=cwd, env=testenv.cmd.generate_env(env))

        # Wait 200ms and check if it is still running
        time.sleep(0.2)
        if daemons[section].poll() is not None:
            testenv.coredump.get_from_coredumpctl()
            raise testenv.NoTraceException(f"program failed to start: {program}")

        # Run setup script
        if "setup" in section_data:
            setup = section_data["setup"]
            logging.info(f"Running {section} setup script")
            if testenv.cmd.run(setup, cwd=cwd, check=False).returncode:
                raise testenv.NoTraceException(f"{section}: setup script failed")


def kill_process_tree(pid, ppids):
    subprocess.run(
        ["kill", "-9", str(pid)],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    for child_pid, child_ppid in ppids:
        if child_ppid == str(pid):
            kill_process_tree(child_pid, ppids)


def kill(pid):
    cmd = ["ps", "-e", "-o", "pid,ppid"]
    ret = subprocess.run(cmd, check=True, stdout=subprocess.PIPE)
    ppids = []
    proc_entries = ret.stdout.decode("utf-8").rstrip().split("\n")[1:]
    for row in proc_entries:
        items = row.split()
        if len(items) != 2:
            raise RuntimeError("Unexpected ps output: " + row)
        ppids.append(items)

    kill_process_tree(pid, ppids)


def check_if_crashed():
    crashed = False
    returncode = None
    for daemon_name, daemon_proc in daemons.items():
        if not testenv.testsuite.is_running(daemon_proc.pid):
            crashed = True
            returncode = daemon_proc.poll()
            break
    if not crashed:
        return

    current_test = testenv.testsuite.get_current_test()
    testenv.coredump.get_from_coredumpctl()

    if current_test:
        logging.error(f"{daemon_name} unexpected exit during {current_test}! rc={returncode}")
        testenv.testsuite.wait_until_test_stopped()
    else:
        logging.error(f"{daemon_name} unexpected exit! rc={returncode}")
    sys.exit(1)


def stop():
    global daemons
    global run_shell_on_stop

    if run_shell_on_stop:
        logging.info("Running interactive shell before stopping daemons (--shell)")

        # stdin=None: override stdin=/dev/null, so we can type into the shell
        testenv.cmd.run(["bash"], cwd=testenv.testdir.testdir, stdin=None, check=False)

        run_shell_on_stop = False

    for daemon in daemons:
        pid = daemons[daemon].pid
        logging.info(f"Stopping {daemon} ({pid})")
        kill(pid)

    daemons = {}
