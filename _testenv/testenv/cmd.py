# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import os.path
import re
import subprocess
import testenv
import testenv.testsuite

env_extra = {"binary_repo": {}, "osmo_dev": {}}
install_dir = {"binary_repo": None, "osmo_dev": None}
make_dir = None
# osmo-dev make dir version, bump when making incompatible changes
make_dir_version = 4


def distro_cache_suffix():
    if not testenv.args.podman or testenv.args.distro == "debian:bookworm":
        return ""
    return f"-{re.sub('[^a-zA-Z0-9]', '-', testenv.args.distro)}"


def init_env_common(envtype):
    """Adjust "Environment variables set by testenv" in README.md when making
    changes here or in other init_env_* functions below."""
    global env_extra

    env_extra[envtype]["TESTENV_INSTALL_DIR"] = install_dir[envtype]

    env_extra[envtype]["CCACHE_DIR"] = testenv.args.ccache
    env_extra[envtype]["TESTENV_CACHE_DIR"] = testenv.args.cache
    env_extra[envtype]["TESTENV_SRC_DIR"] = testenv.src_dir

    env_extra[envtype]["TERM"] = os.environ.get("TERM", "dumb")

    if testenv.args.kernel == "debian":
        env_extra[envtype]["TESTENV_QEMU_KERNEL"] = "debian"
    elif testenv.args.kernel == "custom":
        env_extra[envtype]["TESTENV_QEMU_KERNEL"] = testenv.custom_kernel_path
    if testenv.args.kernel:
        env_extra[envtype]["TESTENV_QEMU_SCRIPTS"] = os.path.join(testenv.data_dir, "scripts/qemu")


def init_env_binary_repo():
    global install_dir

    if testenv.args.podman:
        install_dir["binary_repo"] = "/"
    else:
        install_dir["binary_repo"] = os.path.join(testenv.args.cache, "host/install")

    init_env_common("binary_repo")


def init_env_osmo_dev():
    global env_extra
    global install_dir
    global make_dir

    # install_dir
    if testenv.args.podman:
        install_dir["osmo_dev"] = os.path.join(testenv.args.cache, "podman/install")
        if testenv.args.asan:
            install_dir["osmo_dev"] += "-asan"
        install_dir["osmo_dev"] += distro_cache_suffix()
    else:
        install_dir["osmo_dev"] = os.path.join(testenv.args.cache, "host/install")

    # pkgconfig_path
    pkg_config_path = os.path.join(install_dir["osmo_dev"], "lib/pkgconfig")
    if "PKG_CONFIG_PATH" in os.environ:
        pkg_config_path += f":{os.environ.get('PKG_CONFIG_PATH')}"
    pkg_config_path += ":/usr/lib/pkgconfig"
    env_extra["osmo_dev"]["PKG_CONFIG_PATH"] = pkg_config_path

    # ld_library_path
    ld_library_path = os.path.join(install_dir["osmo_dev"], "lib")
    if "LD_LIBRARY_PATH" in os.environ:
        ld_library_path += f":{os.environ.get('LD_LIBRARY_PATH')}"
    ld_library_path += ":/usr/lib"
    env_extra["osmo_dev"]["LD_LIBRARY_PATH"] = ld_library_path

    # make_dir
    if testenv.args.podman:
        make_dir = os.path.join(testenv.args.cache, "podman", "make")
    else:
        make_dir = os.path.join(testenv.args.cache, "host", "make")
    make_dir += str(make_dir_version)
    if testenv.args.asan:
        make_dir += "-asan"
    make_dir += distro_cache_suffix()
    env_extra["osmo_dev"]["OSMO_DEV_MAKE_DIR"] = make_dir

    init_env_common("osmo_dev")


def exit_error_cmd(completed, error_msg):
    """:param completed: return from run_cmd() below"""

    logging.error(error_msg)
    logging.debug(f"Command: {completed.args}")
    logging.debug(f"Returncode: {completed.returncode}")
    raise RuntimeError("shell command related error, find details right above this python trace")


def generate_env(env=None, podman=False, envtype="binary_repo"):
    env = env or {}
    ret = dict(env_extra[envtype])
    path = os.path.join(testenv.data_dir, "scripts")
    path += f":{os.path.join(testenv.data_dir, 'scripts/qemu')}"
    if testenv.args.action == "run" and testenv.ttcn3_hacks_dir:
        path += f":{os.path.join(testenv.ttcn3_hacks_dir, testenv.args.testsuite)}"

    if install_dir[envtype] and install_dir[envtype] != "/":
        path += f":{os.path.join(install_dir[envtype], 'bin')}"
        path += f":{os.path.join(install_dir[envtype], 'usr/bin')}"

    if podman:
        path += ":/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
    else:
        path += f":{os.environ.get('PATH')}"

    ret["PATH"] = path
    ret["HOME"] = os.environ.get("HOME")
    ret["PYTHONUNBUFFERED"] = "1"

    for var in env:
        ret[var] = env[var]

    # Without podman: pass all environment variables from host (OS#6544)
    if not podman:
        for var in os.environ:
            if var not in ret:
                ret[var] = os.environ.get(var)

    return ret


def run(cmd, check=True, env=None, no_podman=False, stdin=subprocess.DEVNULL, envtype=None, **kwargs):
    env = env or {}

    if not envtype:
        envtype = "binary_repo" if getattr(testenv.args, "binary_repo", True) else "osmo_dev"

    if not no_podman and testenv.args.podman:
        return testenv.podman.exec_cmd(cmd, check=check, env=env, envtype=envtype, **kwargs)

    logging.debug(f"+ {cmd}")

    # Set stdin to /dev/null by default so we can still capture ^C with testenv
    p = subprocess.run(
        cmd,
        env=generate_env(env, envtype=envtype),
        shell=isinstance(cmd, str),
        stdin=stdin,
        check=False,
        **kwargs,
    )

    if p.returncode == 0 or not check:
        return p

    exit_error_cmd(p, "Command failed unexpectedly")
