# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import os.path
import subprocess
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
        pkg_config_path += ":/usr/lib/pkgconfig"

        ld_library_path = os.path.join(usr_dir, "lib")
        if "LD_LIBRARY_PATH" in os.environ:
            ld_library_path += f":{os.environ.get('LD_LIBRARY_PATH')}"
        ld_library_path += ":/usr/lib"

        env_extra["PKG_CONFIG_PATH"] = pkg_config_path
        env_extra["LD_LIBRARY_PATH"] = ld_library_path

    env_extra["CCACHE_DIR"] = testenv.args.ccache
    env_extra["GOCACHE"] = testenv.args.gocache
    env_extra["GOMODCACHE"] = testenv.args.gomodcache
    env_extra["TESTENV_CACHE_DIR"] = testenv.args.cache
    env_extra["TESTENV_SRC_DIR"] = testenv.src_dir

    env_extra["TERM"] = os.environ.get("TERM", "dumb")

    if testenv.args.binary_repo:
        env_extra["TESTENV_GIT_DIR"] = testenv.podman_install.git_dir
    else:
        if testenv.args.podman:
            env_extra["OSMO_DEV_MAKE_DIR"] = os.path.join(testenv.args.cache, "podman", "make")
        else:
            env_extra["OSMO_DEV_MAKE_DIR"] = os.path.join(testenv.args.cache, "host", "make")

    if testenv.args.kernel == "debian":
        env_extra["TESTENV_QEMU_KERNEL"] = "debian"
    elif testenv.args.kernel == "custom":
        env_extra["TESTENV_QEMU_KERNEL"] = testenv.custom_kernel_path
    if testenv.args.kernel:
        env_extra["TESTENV_QEMU_SCRIPTS"] = os.path.join(testenv.data_dir, "scripts/qemu")


def exit_error_cmd(completed, error_msg):
    """:param completed: return from run_cmd() below"""

    logging.error(error_msg)
    logging.debug(f"Command: {completed.args}")
    logging.debug(f"Returncode: {completed.returncode}")
    raise RuntimeError("shell command related error, find details right above this python trace")


def generate_env(env={}, podman=False):
    ret = dict(env_extra)
    path = os.path.join(testenv.data_dir, "scripts")
    path += f":{os.path.join(testenv.data_dir, 'scripts/qemu')}"
    if testenv.testsuite.ttcn3_hacks_dir:
        path += f":{os.path.join(testenv.testsuite.ttcn3_hacks_dir, testenv.args.testsuite)}"

    if usr_dir:
        path += f":{os.path.join(usr_dir, 'bin')}"

    if podman:
        path += ":/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    else:
        path += f":{os.environ.get('PATH')}"

    ret["PATH"] = path
    ret["HOME"] = os.environ.get("HOME")

    for var in env:
        ret[var] = env[var]

    # Without podman: pass all environment variables from host (OS#6544)
    if not podman:
        for var in os.environ:
            if var not in ret:
                ret[var] = os.environ.get(var)

    return ret


def run(cmd, check=True, env={}, no_podman=False, stdin=subprocess.DEVNULL, *args, **kwargs):
    if not no_podman and testenv.args.podman:
        return testenv.podman.exec_cmd(cmd, check=check, env=env, *args, **kwargs)

    logging.debug(f"+ {cmd}")

    # Set stdin to /dev/null by default so we can still capture ^C with testenv
    p = subprocess.run(
        cmd,
        env=generate_env(env),
        shell=isinstance(cmd, str),
        stdin=stdin,
        *args,
        **kwargs,
    )

    if p.returncode == 0 or not check:
        return p

    exit_error_cmd(p, "Command failed unexpectedly")
