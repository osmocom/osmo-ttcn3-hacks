import logging
import os
import os.path
import subprocess
import sys
import tempfile
import testenv
import testenv.testsuite

env_extra = {}
usr_dir = None


def init_env():
    global env_extra
    global usr_dir

    if testenv.args.podman:
        if not testenv.args.binary_repo:
            usr_dir = os.path.join(testenv.args.cache, "podman", "usr")
    else:
        usr_dir = os.path.join(testenv.args.cache, "host", "usr")

    if usr_dir:
        pkg_config_path = os.path.join(usr_dir, "lib/pkgconfig")
        if "PKG_CONFIG_PATH" in os.environ:
            pkg_config_path += f":{os.environ.get('PKG_CONFIG_PATH')}"
        pkg_config_path += f":/usr/lib/pkgconfig"

        ld_library_path = os.path.join(usr_dir, "lib")
        if "LD_LIBRARY_PATH" in os.environ:
            ld_library_path += f":{os.environ.get('LD_LIBRARY_PATH')}"
        ld_library_path += f":/usr/lib"

        env_extra["PKG_CONFIG_PATH"] = pkg_config_path
        env_extra["LD_LIBRARY_PATH"] = ld_library_path

    env_extra["CCACHE_DIR"] = testenv.args.ccache
    env_extra["TESTENV_CACHE_DIR"] = testenv.args.cache

    env_extra["TERM"] = os.environ.get("TERM", "dumb")

    if testenv.args.podman:
        env_extra["FAKE_TRX_DIR"] = os.path.join(testenv.podman_install.bb_dir,
                                                 "src/target/trx_toolkit")
    else:
        env_extra["FAKE_TRX_DIR"] = os.path.join(testenv.top_dir, "osmocom-bb",
                                                 "src/target/trx_toolkit")


def exit_error_cmd(completed, error_msg):
    """ :param completed: return from run_cmd() below """

    logging.error(error_msg)
    logging.debug(f"Command: {completed.args}")
    logging.debug(f"Returncode: {completed.returncode}")
    raise RuntimeError("shell command related error, find details right above"
                       " this python trace")


def generate_env(env={}, podman=False):
    ret = dict(env_extra)
    path = ""

    if usr_dir:
        path = os.path.join(usr_dir, "bin")
        if testenv.testsuite.ttcn3_hacks_dir:
            path += f":{os.path.join(testenv.testsuite.ttcn3_hacks_dir, testenv.args.testsuite)}"
        path += f":{os.path.join(testenv.data_dir, 'scripts')}"

    if podman:
        path += f":/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    else:
        path += f":{os.environ.get('PATH')}"

    ret["PATH"] = path

    for var in env:
        ret[var] = env[var]
    return ret


def run(cmd, check=True, env={}, no_podman=False, *args, **kwargs):
    if not no_podman and testenv.podman.is_running():
        return testenv.podman.exec_cmd(cmd, check=check, env=env, *args, **kwargs)

    logging.debug(f"+ {cmd}")

    p = subprocess.run(cmd, env=generate_env(env), shell=isinstance(cmd, str),
                       *args, **kwargs)

    if p.returncode == 0 or not check:
        return p

    exit_error_cmd(p, "Command failed unexpectedly")
