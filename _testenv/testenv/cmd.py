# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import os.path
import subprocess
import testenv
import testenv.testsuite

env_extra = {}
install_dir = None
make_dir = None
# osmo-dev make dir version, bump when making incompatible changes
make_dir_version = 3


def init_env():
    """Adjust "Environment variables set by testenv" in README.md when making
    changes here."""
    global env_extra
    global install_dir
    global make_dir

    if testenv.args.podman:
        if testenv.args.binary_repo:
            install_dir = "/"
        else:
            install_dir = os.path.join(testenv.args.cache, "podman/install")
    else:
        install_dir = os.path.join(testenv.args.cache, "host/install")

    if not testenv.args.binary_repo:
        pkg_config_path = os.path.join(install_dir, "lib/pkgconfig")
        if "PKG_CONFIG_PATH" in os.environ:
            pkg_config_path += f":{os.environ.get('PKG_CONFIG_PATH')}"
        pkg_config_path += ":/usr/lib/pkgconfig"

        ld_library_path = os.path.join(install_dir, "lib")
        if "LD_LIBRARY_PATH" in os.environ:
            ld_library_path += f":{os.environ.get('LD_LIBRARY_PATH')}"
        ld_library_path += ":/usr/lib"

        env_extra["PKG_CONFIG_PATH"] = pkg_config_path
        env_extra["LD_LIBRARY_PATH"] = ld_library_path

    env_extra["CCACHE_DIR"] = testenv.args.ccache
    env_extra["TESTENV_CACHE_DIR"] = testenv.args.cache
    env_extra["TESTENV_SRC_DIR"] = testenv.src_dir
    env_extra["TESTENV_INSTALL_DIR"] = install_dir

    env_extra["TERM"] = os.environ.get("TERM", "dumb")

    if not testenv.args.binary_repo:
        if testenv.args.podman:
            make_dir = os.path.join(testenv.args.cache, "podman", "make")
        else:
            make_dir = os.path.join(testenv.args.cache, "host", "make")
        make_dir += str(make_dir_version)
        env_extra["OSMO_DEV_MAKE_DIR"] = make_dir

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
    if testenv.args.action == "run" and testenv.testsuite.ttcn3_hacks_dir:
        path += f":{os.path.join(testenv.testsuite.ttcn3_hacks_dir, testenv.args.testsuite)}"

    if install_dir and install_dir != "/":
        path += f":{os.path.join(install_dir, 'bin')}"

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
