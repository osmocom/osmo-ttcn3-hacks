# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import atexit
import datetime
import json
import logging
import multiprocessing
import os
import shlex
import subprocess
import testenv.cmd
import testenv.testdir
import testenv.coredump
import time

image_name = None
distro = None
container_name = None  # instance of image
apt_dir_var_cache = None
apt_dir_var_lib = None
feed_watchdog_process = None
run_shell_on_stop = False
restart_count = 0


def image_exists():
    return testenv.cmd.run(["podman", "image", "exists", image_name], check=False, no_podman=True).returncode == 0


def image_up_to_date():
    history = testenv.cmd.run(
        ["podman", "history", image_name, "--format", "json"],
        capture_output=True,
        no_podman=True,
        text=True,
    )
    created = json.loads(history.stdout)[0]["created"].split(".", 1)[0]
    created = datetime.datetime.strptime(created, "%Y-%m-%dT%H:%M:%S")
    logging.debug(f"Image creation date: {created}")

    # On a local development system, we can say that the podman image is
    # outdated if the Dockerfile is newer than the creation date. But this does
    # not work for jenkins where Dockerfile may just be from a new git
    # checkout. So allow to skip this check.
    if os.environ.get("TESTENV_NO_IMAGE_UP_TO_DATE_CHECK"):
        logging.debug("Assuming the podman image is up-to-date")
        return True

    dockerfile = os.path.join(testenv.data_dir, "podman/Dockerfile")
    mtime = os.stat(dockerfile).st_mtime
    mtime = datetime.datetime.utcfromtimestamp(mtime)
    logging.debug(f"Dockerfile last modified: {str(mtime).split('.')[0]}")

    return mtime < created


def image_build(check_existing=True):
    if check_existing:
        if image_exists() and image_up_to_date():
            logging.debug(f"Podman image is up-to-date: {image_name}")
            if testenv.args.force:
                logging.debug("Building anyway since --force was used")
            else:
                return

    logging.info(f"Building podman image: {image_name}")
    testenv.cmd.run(
        [
            "buildah",
            "build",
            "--build-arg",
            f"DISTRO={distro}",
            "-t",
            image_name,
            os.path.join(testenv.data_dir, "podman"),
        ],
        no_podman=True,
    )


def generate_env_podman(env={}):
    ret = []

    for key, val in testenv.cmd.generate_env(env, True).items():
        ret += ["-e", f"{key}={val}"]

    return ret


def init_image_name_distro():
    global image_name
    global distro

    distro = getattr(testenv.args, "distro", testenv.distro_default)
    image_name = f"{distro}-osmo-ttcn3-testenv"
    image_name = image_name.replace(":", "-").replace("_", "-")


def init():
    global apt_dir_var_cache
    global apt_dir_var_lib
    global run_shell_on_stop

    apt_dir_var_cache = os.path.join(testenv.args.cache, "podman", "var-cache-apt")
    apt_dir_var_lib = os.path.join(testenv.args.cache, "podman", "var-lib-apt")

    os.makedirs(apt_dir_var_cache, exist_ok=True)
    os.makedirs(apt_dir_var_lib, exist_ok=True)
    os.makedirs(testenv.args.ccache, exist_ok=True)

    init_image_name_distro()

    if not image_exists():
        raise testenv.NoTraceException("Missing podman image, run 'testenv.py init podman' first to build it")
    if not image_up_to_date():
        if os.environ.get("TESTENV_REBUILD_OUTDATED_IMAGE") == "1":
            logging.warning("The podman image is outdated, rebuilding it... (TESTENV_REBUILD_OUTDATED_IMAGE=1)")
            image_build(False)
        else:
            # Rebuilding the image takes some time, and oftentimes it is not
            # needed. So by default don't force the user to rebuild it, just
            # show a warning.
            logging.warning(
                "The podman image might be outdated, consider running 'testenv.py init podman' to rebuild it"
            )
            logging.debug("Set TESTENV_REBUILD_OUTDATED_IMAGE=1 to rebuild it automatically")

    atexit.register(stop)

    if testenv.args.shell:
        run_shell_on_stop = True


def exec_cmd(cmd, podman_opts=[], cwd=None, env={}, *args, **kwargs):
    if not container_name:
        raise RuntimeError(f"Attempting to execute a command in podman, but the container isn't running anymore: {cmd}")

    podman_opts = list(podman_opts)
    podman_opts += generate_env_podman(env)
    # Attach a fake tty (eclipse-titan won't print colored output otherwise)
    podman_opts += ["-t"]

    if cwd:
        podman_opts += ["-w", cwd]

    if isinstance(cmd, str):
        cmd = ["sh", "-c", cmd]

    return testenv.cmd.run(
        ["podman", "exec"] + podman_opts + [container_name] + cmd,
        no_podman=True,
        *args,
        **kwargs,
    )


def exec_cmd_background(cmd, podman_opts=[], cwd=None, env={}):
    podman_opts = list(podman_opts) + generate_env_podman(env)

    if cwd:
        podman_opts += ["-w", cwd]

    if isinstance(cmd, str):
        cmd = ["sh", "-c", cmd]

    cmd = ["podman", "exec"] + podman_opts + [container_name] + cmd
    logging.debug(f"+ {cmd}")

    return subprocess.Popen(cmd)


def feed_watchdog_loop():
    # The script testenv-podman-main.sh checks every 10s for /tmp/watchdog and
    # deletes the file. Create it here every 5s so the container keeps running.
    # This ensures that if we run in jenkins and the job gets aborted, the
    # container will terminate after a few seconds.
    try:
        while True:
            time.sleep(2)
            p = subprocess.run(["podman", "exec", container_name, "touch", "/tmp/watchdog"], stderr=subprocess.DEVNULL)
            if p.returncode:
                logging.debug("feed_watchdog_loop: podman container has stopped")
                return
    except KeyboardInterrupt:
        pass


def wait_until_started():
    for i in range(100):
        time.sleep(0.1)
        if is_running():
            return
    raise RuntimeError("Podman failed to start")


def start_in_background(cmd):
    log_dir = os.path.join(testenv.testdir.testdir_topdir, "podman")
    os.makedirs(log_dir, exist_ok=True)

    logging.debug(f"+ {cmd}")
    subprocess.Popen(cmd, env=testenv.cmd.generate_env())

    wait_until_started()

    feed_watchdog_process = multiprocessing.Process(target=feed_watchdog_loop)
    feed_watchdog_process.start()


def start():
    global container_name
    global feed_watchdog_process

    testdir_topdir = testenv.testdir.testdir_topdir
    osmo_dev_dir = testenv.osmo_dev.get_osmo_dev_dir()
    container_name = f"{testenv.testdir.prefix}-{restart_count}"

    # Custom seccomp profile that allows io_uring
    seccomp = os.path.join(testenv.data_dir, "podman/seccomp.json")

    cmd = [
        "podman",
        "run",
        "--rm",
        "--name",
        container_name,
        "--log-driver",
        "json-file",
        "--log-opt",
        f"path={testdir_topdir}/podman/{container_name}.log",
        f"--security-opt=seccomp={seccomp}",
        "--cap-add=NET_ADMIN",  # for dumpcap, tun devices, osmo-pcap-client
        "--cap-add=NET_RAW",  # for dumpcap, osmo-pcap-client
        "--device=/dev/net/tun",  # for e.g. ggsn_tests
        "--volume",
        f"{apt_dir_var_cache}:/var/cache/apt",
        "--volume",
        f"{apt_dir_var_lib}:/var/lib/apt",
        "--sysctl",
        "net.ipv4.conf.all.send_redirects=0",  # OS#6575
        "--sysctl",
        "net.ipv4.conf.default.send_redirects=0",  # OS#6575
        "-e",
        "PODMAN=1",
    ]

    if testenv.args.binary_repo:
        cmd += [
            "-e",
            "TESTENV_BINARY_REPO=1",
        ]
    else:
        cmd += [
            "--volume",
            f"{osmo_dev_dir}:{osmo_dev_dir}",
        ]

    if testenv.args.kernel:
        if not os.environ.get("TESTENV_NO_KVM") and os.path.exists("/dev/kvm"):
            cmd += ["--volume", "/dev/kvm:/dev/kvm"]
        if os.path.islink(testenv.custom_kernel_path):
            dest = os.readlink(testenv.custom_kernel_path)
            cmd += ["--volume", f"{dest}:{dest}:ro"]

    cmd += [
        "--volume",
        f"{testdir_topdir}:{testdir_topdir}",
        "--volume",
        f"{testenv.args.cache}:{testenv.args.cache}",
        "--volume",
        f"{testenv.args.ccache}:{testenv.args.ccache}",
        "--volume",
        f"{testenv.src_dir}:{testenv.src_dir}",
        image_name,
        os.path.join(testenv.data_dir, "scripts/testenv-podman-main.sh"),
    ]

    start_in_background(cmd)

    exec_cmd(["rm", "/etc/apt/apt.conf.d/docker-clean"])

    pkgcache = os.path.join(apt_dir_var_cache, "pkgcache.bin")
    if not os.path.exists(pkgcache):
        exec_cmd(["apt-get", "-q", "update"])


def distro_to_repo_dir(distro):
    if distro == "debian:bookworm":
        return "Debian_12"
    raise RuntimeError(f"Can't translate distro {distro} to repo_dir!")


def enable_binary_repo():
    config = "deb [signed-by=/obs.key]"
    config += " https://downloads.osmocom.org/packages/"
    config += testenv.args.binary_repo.replace(":", ":/")
    config += "/"
    config += distro_to_repo_dir(distro)
    config += "/ ./"

    path = "/etc/apt/sources.list.d/osmocom.list"

    exec_cmd(["sh", "-c", f"echo {shlex.quote(config)} > {path}"])
    exec_cmd(["apt-get", "-q", "update"])


def is_running():
    if container_name is None:
        return False

    cmd = ["podman", "ps", "-q", "--filter", f"name={container_name}"]
    if not subprocess.run(cmd, capture_output=True, text=True).stdout:
        return False

    return True


def stop(restart=False):
    global container_name
    global run_shell_on_stop
    global restart_count
    global feed_watchdog_process

    if not is_running():
        return

    # If we have a coredump, we must get the backtrace by running gdb inside
    # the container. So do it before stopping the container.
    testenv.coredump.get_backtrace()

    if not restart and run_shell_on_stop:
        logging.info("Running interactive shell before stopping container (--shell)")

        # stdin=None: override stdin=/dev/null, so we can type into the shell
        exec_cmd(["bash"], ["-i"], cwd=testenv.testdir.testdir, stdin=None, check=False)

        run_shell_on_stop = False

    restart_msg = " (restart)" if restart else ""
    logging.info(f"Stopping podman container{restart_msg}")

    if feed_watchdog_process:
        feed_watchdog_process.terminate()
        feed_watchdog_process.wait()
        feed_watchdog_process = None

    testenv.cmd.run(["podman", "kill", container_name], no_podman=True, check=False)

    container_name = None

    if restart:
        restart_count += 1
        start()
