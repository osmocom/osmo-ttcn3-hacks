import atexit
import datetime
import json
import logging
import os
import shlex
import subprocess
import testenv.cmd
import testenv.testdir
import time

image_name = None
distro = None
container_name = None  # instance of image
apt_dir_var_cache = None
apt_dir_var_lib = None


def image_exists():
    return testenv.cmd.run(["podman", "image", "exists", image_name],
                           check=False).returncode == 0


def image_up_to_date():
    history = testenv.cmd.run(["podman", "history", image_name, "--format", "json"],
                              capture_output=True, text=True)
    created = json.loads(history.stdout)[0]["created"].split(".",1)[0]
    created = datetime.datetime.strptime(created, "%Y-%m-%dT%H:%M:%S")
    logging.debug(f"Image creation date: {created}")

    dockerfile = os.path.join(testenv.data_dir, "podman/Dockerfile")
    mtime = os.stat(dockerfile).st_mtime
    mtime = datetime.datetime.utcfromtimestamp(mtime)
    logging.debug(f"Dockerfile last modified: {str(mtime).split('.')[0]}")

    return mtime < created


def image_build():
    if image_exists() and image_up_to_date():
        logging.debug(f"Podman image is up-to-date: {image_name}")
        if testenv.args.force:
            logging.debug("Building anyway since --force was used")
        else:
            return

    logging.info(f"Building podman image: {image_name}")
    testenv.cmd.run(["buildah", "build",
                     "--build-arg", f"DISTRO={distro}",
                     "-t", image_name,
                     os.path.join(testenv.data_dir, "podman")])


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
    image_name = image_name.replace(":","-").replace("_","-")


def init():
    global apt_dir_var_cache
    global apt_dir_var_lib

    apt_dir_var_cache = os.path.join(testenv.args.cache, "podman", "var-cache-apt")
    apt_dir_var_lib = os.path.join(testenv.args.cache, "podman", "var-lib-apt")

    os.makedirs(apt_dir_var_cache, exist_ok=True)
    os.makedirs(apt_dir_var_lib, exist_ok=True)
    os.makedirs(testenv.args.ccache, exist_ok=True)

    init_image_name_distro()

    if not image_exists():
        raise testenv.NoTraceException("Missing podman image, run 'testenv.py"
                                       " image' first to build it")
    if not image_up_to_date():
        logging.warning("The podman image is outdated, consider running"
                        " 'testenv.py image' to rebuild it")

    atexit.register(stop)


def exec_cmd(cmd, podman_opts=[], cwd=None, env={}, *args, **kwargs):
    podman_opts = list(podman_opts) + generate_env_podman(env)

    if cwd:
        podman_opts += ["-w", cwd]

    if isinstance(cmd, str):
        cmd = ["sh", "-c", cmd]

    testenv.cmd.run(["podman", "exec"]
                    + podman_opts
                    + [container_name]
                    + cmd,
                    no_podman=True,
                    *args, **kwargs)


def exec_cmd_background(cmd, podman_opts=[], cwd=None, env={}):
    podman_opts = list(podman_opts) + generate_env_podman(env)

    if cwd:
        podman_opts += ["-w", cwd]

    if isinstance(cmd, str):
        cmd = ["sh", "-c", cmd]

    cmd = ["podman", "exec"] + podman_opts + [container_name] + cmd
    logging.debug(f"+ {cmd}")

    return subprocess.Popen(cmd)


def start():
    global container_name

    testdir_topdir = testenv.testdir.testdir_topdir
    container_name = os.path.basename(testdir_topdir)
    cmd = [
        "podman", "run",
        "--rm",
        "--name", container_name,
        "--interactive",
        "--tty",
        "--detach",
        "--cap-add=NET_ADMIN",  # for dumpcap, tun devices, osmo-pcap-client
        "--cap-add=NET_RAW",  # for dumpcap, osmo-pcap-client
        "--device=/dev/net/tun",  # for e.g. ggsn_tests
        "--volume", f"{apt_dir_var_cache}:/var/cache/apt",
        "--volume", f"{apt_dir_var_lib}:/var/lib/apt",
        "--volume", f"{testenv.top_dir}:{testenv.top_dir}",
        "--volume", f"{testenv.args.cache}:{testenv.args.cache}",
        "--volume", f"{testenv.args.ccache}:{testenv.args.ccache}",
        "--volume", f"{testdir_topdir}:{testdir_topdir}",
    ]

    # if osmo-dev is a symlink, mount its target too
    osmo_dev_dir = os.path.join(testenv.top_dir, "osmo-dev")
    if os.path.islink(osmo_dev_dir):
        realpath = os.path.realpath(osmo_dev_dir)
        cmd += [
            "--volume", f"{realpath}:{realpath}",
        ]

    cmd += [
        image_name,
        "bash"
    ]

    testenv.cmd.run(cmd, no_podman=True)
    exec_cmd(["rm", "/etc/apt/apt.conf.d/docker-clean"])

    pkgcache = os.path.join(apt_dir_var_cache, "pkgcache.bin")
    if not os.path.exists(pkgcache):
        exec_cmd(["apt-get", "update"])


def enable_binary_repo():
    config = "deb [signed-by=/obs.key]"
    config += " https://downloads.osmocom.org/packages/"
    config += testenv.args.binary_repo.replace(":", ":/")
    config += "/"
    config += testenv.args.distro.replace(":", "_").title()
    config += "/ ./"

    path = "/etc/apt/sources.list.d/osmocom.list"

    exec_cmd(["sh", "-c", f"echo {shlex.quote(config)} > {path}"])
    exec_cmd(["apt-get", "update"])


def is_running():
    return container_name is not None


def stop():
    if not is_running():
        return

    if testenv.args.shell:
        try:
            logging.info("Running interactive shell before stopping container (--shell)")
            exec_cmd(["bash"], ["-it"], cwd=testenv.testdir.testdir)
        except:
            pass

    logging.info("Stopping podman container")
    testenv.cmd.run(["podman", "kill", container_name], no_podman=True)
