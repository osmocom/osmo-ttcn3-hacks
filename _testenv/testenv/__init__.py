# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import argparse
import logging
import os.path


class NoTraceException(Exception):
    pass


args = None

src_dir = os.environ.get("TESTENV_SRC_DIR", os.path.realpath(f"{__file__}/../../../.."))
data_dir = os.path.join(os.path.realpath(f"{__file__}/../.."), "data")
custom_kernel_path = os.path.join(os.path.realpath(f"{__file__}/../../.."), ".linux")
distro_default = "debian:bookworm"
cache_dir_default = os.path.join(os.path.expanduser("~/.cache"), "osmo-ttcn3-testenv")
ccache_dir_default = os.path.join(cache_dir_default, "ccache")

log_prefix = "[testenv]"


def resolve_testsuite_name_alias(name):
    mapping = {
        "ggsn": "ggsn_tests",
    }

    if name in mapping:
        logging.debug(f"Using testsuite {mapping[name]} (via alias {name})")
        return mapping[name]

    return name


def parse_args():
    global args
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Build/install everything for a testsuite and run it.\n"
        "\n"
        "examples:\n"
        "  ./testenv.py run mgw\n"
        "  ./testenv.py run mgw --test TC_crcx\n"
        "  ./testenv.py run mgw --podman --binary-repo osmocom:latest\n"
        "  ./testenv.py run mgw --podman --binary-repo osmocom:nightly\n"
        "  ./testenv.py run mgw --podman --binary-repo osmocom:nightly:asan\n"
        "  ./testenv.py run mgw --io-uring\n"
        "  ./testenv.py run ggsn --config open5gs\n"
        "  ./testenv.py run ggsn --config 'osmo_ggsn_v4_only'\n"
        "  ./testenv.py run ggsn --config 'osmo_ggsn*'\n",
    )

    sub = parser.add_subparsers(title="action", dest="action", required=True)

    sub_init = sub.add_parser("init", help="initialize osmo-dev/podman")
    sub_init_runtime = sub_init.add_subparsers(title="runtime", required=True, dest="runtime")
    sub_init_runtime.add_parser(
        "osmo-dev",
        help="prepare osmo-dev (top-level makefile scripts, for building test"
        " components from source when using 'run' without '--binary-repo')",
    )

    sub_podman = sub_init_runtime.add_parser("podman", help="prepare the podman image (for 'run --podman')")
    sub_podman.add_argument(
        "-f",
        "--force",
        action="store_true",
        help="build image even if it is up-to-date",
    )

    sub_run = sub.add_parser("run", help="build components and run a testsuite")

    group = sub_run.add_argument_group("testsuite options")
    group.add_argument("testsuite", help="a directory in osmo-ttcn3-hacks.git (msc, bsc, mgw, ...)")
    group.add_argument(
        "-t",
        "--test",
        help="only run one specific test (e.g. TC_selftest, BTS_Tests_OML.TC_wrong_mdisc)",
    )
    group.add_argument(
        "-c",
        "--config",
        action="append",
        help="which testenv.cfg to use (supports * wildcards via fnmatch)",
    )
    group.add_argument("-i", "--io-uring", action="store_true", help="set LIBOSMO_IO_BACKEND=IO_URING")

    group = sub_run.add_argument_group("source/binary options", "All components are built from source by default.")
    group.add_argument(
        "-b",
        "--binary-repo",
        metavar="OBS_PROJECT",
        help="use binary packages from this Osmocom OBS project instead (e.g. osmocom:nightly)",
    )
    group.add_argument(
        "-j",
        "--jobs",
        help="number of jobs to run simultaneously (default: nproc)",
        type=int,
    )

    group = sub_run.add_argument_group("exit options", "When and how testenv should exit when done.")
    group = group.add_mutually_exclusive_group()
    group.add_argument(
        "-B",
        "--bisect",
        action="store_true",
        help="exit with != 0 if at least one test failed (use with git bisect)",
    )
    group.add_argument(
        "-u",
        "--until-nok",
        action="store_true",
        help="run until there was either a failure or error",
    )

    group = sub_run.add_argument_group(
        "QEMU options",
        "For some tests, the SUT can or must run in QEMU, typically to use kernel GTP-U.",
    )
    group = group.add_mutually_exclusive_group()
    group.add_argument(
        "-D",
        "--debian-kernel",
        action="store_const",
        dest="kernel",
        const="debian",
        help="run SUT in QEMU with debian kernel",
    )
    group.add_argument(
        "-C",
        "--custom-kernel",
        action="store_const",
        dest="kernel",
        const="custom",
        help=f"run SUT in QEMU with custom kernel ({custom_kernel_path})",
    )

    group = sub_run.add_argument_group(
        "config file options",
        "Testsuite and test component configs"
        " for nightly/master versions of test"
        " components are used, unless a binary"
        " repository ending in :latest is set"
        " or --latest is used.",
    )
    group.add_argument("--latest", action="store_true", help="use latest configs")

    group = sub_run.add_argument_group("podman options", "All components are run directly on the host by default.")
    group.add_argument("-p", "--podman", action="store_true", help="run all components inside podman")
    group.add_argument(
        "-d",
        "--distro",
        default=distro_default,
        help=f"distribution for podman (default: {distro_default})",
    )
    group.add_argument(
        "-s",
        "--shell",
        action="store_true",
        help="run an interactive shell before stopping daemons/container",
    )

    group = sub_run.add_argument_group("output options")
    group.add_argument("-l", "--log-dir", help="log here instead of a random dir in /tmp")
    group.add_argument(
        "-n",
        "--no-tee",
        dest="tee",
        action="store_false",
        help="don't send test component's output to stdout",
    )

    group = sub_run.add_argument_group("cache options")
    group.add_argument(
        "--cache",
        help=f"cache path (default: {cache_dir_default})",
        default=cache_dir_default,
    )
    group.add_argument(
        "--ccache",
        help=f"ccache path (default: {ccache_dir_default})",
        default=ccache_dir_default,
    )

    sub.add_parser("clean", help="clean previous build artifacts")

    args = parser.parse_args()

    if args.action == "run":
        args.testsuite = resolve_testsuite_name_alias(args.testsuite)
        if args.binary_repo and args.binary_repo.endswith(":latest"):
            logging.debug("Binary repository ends in :latest, using latest configs")
            args.latest = True
    else:
        # podman is only used in "testenv run"
        args.podman = False


def verify_args_run():
    if args.action != "run":
        return

    if args.binary_repo and not args.podman:
        raise NoTraceException("--binary-repo requires --podman")

    if args.kernel == "debian" and not args.podman:
        raise NoTraceException("--kernel-debian requires --podman")

    if args.kernel == "custom" and not os.path.exists(custom_kernel_path):
        logging.critical(
            "See _testenv/README.md for more information on downloading a pre-built kernel or building your own kernel."
        )
        raise NoTraceException(f"For --kernel-custom, put a symlink or copy of your kernel to: {custom_kernel_path}")

    ttcn3_hacks_dir_src = os.path.realpath(f"{__file__}/../../..")
    testsuite_dir = os.path.join(ttcn3_hacks_dir_src, args.testsuite)
    if not os.path.exists(testsuite_dir):
        raise NoTraceException(f"testsuite dir not found: {testsuite_dir}")


def init_args():
    parse_args()
    verify_args_run()


class ColorFormatter(logging.Formatter):
    colors = {
        "debug": "\033[37m",  # light gray
        "info": "\033[94m",  # blue
        "warning": "\033[93m",  # yellow
        "error": "\033[91m",  # red
        "critical": "\033[91m",  # red
        "reset": "\033[0m",
    }

    def __init__(self):
        for color in self.colors.keys():
            env_var = f"TESTENV_COLOR_{color.upper()}"
            if env_var in os.environ:
                self.colors[color] = os.environ.get(env_var)

        super().__init__()

    def format(self, record):
        if record.levelno == logging.DEBUG:
            color = "debug"
        elif record.levelno == logging.INFO:
            color = "info"
        elif record.levelno == logging.WARNING:
            color = "warning"
        elif record.levelno == logging.ERROR:
            color = "error"
        elif record.levelno == logging.CRITICAL:
            color = "critical"

        self._style._fmt = f"{self.colors[color]}{log_prefix} %(msg)s{self.colors['reset']}"
        result = logging.Formatter.format(self, record)

        return result


def init_logging():
    formatter = ColorFormatter()
    root_logger = logging.getLogger()
    root_logger.setLevel(logging.DEBUG)
    root_logger.handlers = []

    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    root_logger.addHandler(handler)


def set_log_prefix(new):
    global log_prefix

    log_prefix = new
