import atexit
import logging
import os
import os.path
import shlex
import subprocess
import testenv
import testenv.testdir
import time

daemons = {}


def init():
    if not testenv.args.podman:
        atexit.register(stop)


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
        logging.info(f"Starting daemon: {section} ({program})")

        cwd = os.path.join(testenv.testdir.testdir, section)
        os.makedirs(cwd, exist_ok=True)

        log = os.path.join(testenv.testdir.testdir, section, f"{section}.log")
        cmd = ["sh", "-c", f"{program} 2>&1 | tee {shlex.quote(log)}"]

        if testenv.podman.is_running():
            daemons[section] = testenv.podman.exec_cmd_background(cmd, cwd=cwd)
        else:
            logging.debug(f"+ {cmd}")
            daemons[section] = subprocess.Popen(cmd, cwd=cwd,
                                                env=testenv.cmd.generate_env())

        # Wait 200ms and check if it is still running
        time.sleep(0.2)
        if daemons[section].poll() is not None:
            raise testenv.NoTraceException(f"program failed to start: {program}")

        # Run setup script
        if "setup" in section_data:
            setup = section_data["setup"]
            logging.info(f"Running setup script: {section} ({setup})")
            testenv.cmd.run(setup, cwd=cwd)


def kill_process_tree(pid, ppids):
    subprocess.run(["kill", "-9", str(pid)], check=False)

    for (child_pid, child_ppid) in ppids:
        if child_ppid == str(pid):
            kill_process_tree(child_pid, ppids)


def kill(pid):
    cmd = ["ps", "-e", "-o", "pid,ppid"]
    ret = subprocess.run(cmd, check=True, stdout=subprocess.PIPE)
    ppids = []
    proc_entries = ret.stdout.decode("utf-8").rstrip().split('\n')[1:]
    for row in proc_entries:
        items = row.split()
        if len(items) != 2:
            raise RuntimeError("Unexpected ps output: " + row)
        ppids.append(items)

    kill_process_tree(pid, ppids)


def stop():
    global daemons

    if testenv.args.shell:
        logging.info("Running interactive shell before stopping daemons (--shell)")
        testenv.cmd.run(["bash"], cwd=testenv.testdir.testdir)

    for daemon in daemons:
        pid = daemons[daemon].pid
        logging.info(f"Stopping daemon: {daemon} ({pid})")
        kill(pid)

    daemons = {}
