# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import configparser
import fnmatch
import glob
import logging
import os.path
import sys
import testenv
import testenv.testsuite

cfgs = {}
current = None


def set_current(cfg_name, loop_count=0):
    global current
    current = cfg_name
    loop_str = ""

    if testenv.args.until_nok:
        loop_str = f"[loop-{loop_count}]"

    if cfg_name == "testenv.cfg":
        testenv.set_log_prefix(f"[testenv]{loop_str}")
    else:
        cfg_name = cfg_name.replace("testenv_", "")
        cfg_name = cfg_name.replace(".cfg", "")
        testenv.set_log_prefix(f"[testenv]{loop_str}[{cfg_name}]")


def exit_error_readme():
    readme = os.path.join(testenv.testsuite.ttcn3_hacks_dir, "_testenv/README.md")
    logging.error(f"More information: {readme}")
    sys.exit(1)


def handle_latest(cfg, path):
    """Remove _latest keys from cfg or use them instead of the regular keys,
    if --latest is set."""

    for section in cfg:
        for key in cfg[section]:
            if not key.endswith("_latest"):
                continue

            if testenv.args.latest:
                key_regular = key.replace("_latest", "")
                logging.debug(f"{path}: [{section}]: using {key} instead of {key_regular} (--latest is set)")
                cfg[section][key_regular] = cfg[section][key]
            else:
                logging.debug(f"{path}: [{section}]: ignoring {key} (--latest is not set)")

            del cfg[section][key]


def get_vty_host_port(cfg, path=None):
    host = "127.0.0.1"
    port = None

    for section in cfg:
        if "vty_port" in cfg[section]:
            if port:
                logging.error(f"Error in {path}, section {section}:")
                logging.error("  Found vty_port in multiple sections. This is not supported.")
                sys.exit(1)
            port = cfg[section]["vty_port"]
        if "vty_host" in cfg[section]:
            if not port:
                logging.error(f"Error in {path}, section {section}:")
                logging.error("  Found vty_host in section without vty_port.")
                sys.exit(1)
            host = cfg[section]["vty_host"]

    return host, port


def verify_qemu_cfgs():
    """Check if passed -C or -K args make sense with the testenv configs."""
    testsuite = testenv.args.testsuite
    qemu_supported = False
    qemu_required = False

    for basename, cfg in cfgs.items():
        for section in cfg.keys():
            if "qemu" in cfg[section]:
                qemu_supported = True
                if cfg[section]["qemu"] == "required":
                    qemu_required = True
                break

        if testenv.args.kernel and not qemu_supported:
            logging.critical(f"{testsuite}/{basename}: doesn't support running in QEMU")
            exit_error_readme()

        if not testenv.args.kernel and qemu_required:
            logging.error(f"{testsuite}/{basename}: {section} must run in QEMU")
            logging.error("Use one of:")
            logging.error("  -D, --debian-kernel")
            logging.error("  -C, --custom-kernel")
            exit_error_readme()


def verify_qemu_section(path, cfg, section):
    """Verify that qemu= has proper values."""
    if "qemu" not in cfg[section]:
        return

    valid = ["optional", "required"]
    value = cfg[section]["qemu"]

    if value not in valid:
        logging.error(f"{path}: [{section}]: qemu={value} is invalid, must be one of: {valid}")
        exit_error_readme()


def verify(cfg, path):
    keys_valid_testsuite = [
        "clean",
        "config",
        "copy",
        "prepare",
        "program",
    ]
    keys_valid_component = [
        "clean",
        "copy",
        "make",
        "package",
        "prepare",
        "program",
        "qemu",
        "setup",
        "vty_host",
        "vty_port",
    ]
    keys_invalid = {
        "configs": "config",
        "packages": "package",
        "programs": "program",
    }
    keys_lists = [
        "copy",
        "package",
    ]

    if "testsuite" not in cfg:
        logging.error(f"{path}: missing [testsuite] section")
        exit_error_readme()
    if "program" not in cfg["testsuite"]:
        logging.error(f"{path}: missing program= in [testsuite]")
        exit_error_readme()
    if " " in cfg["testsuite"]["program"]:
        logging.error(f"{path}: program= in [testsuite] must not have arguments")
        exit_error_readme()
    if " " in cfg["testsuite"]["config"]:
        logging.error(f"{path}: config= in [testsuite] must not have spaces")
        exit_error_readme()
    if "config" not in cfg["testsuite"]:
        logging.error(f"{path}: missing config= in [testsuite]")
        exit_error_readme()

    for section in cfg:
        for key in cfg[section].keys():
            valid = keys_valid_component
            if section == "testsuite":
                valid = keys_valid_testsuite

            if key in valid:
                continue

            msg = f"{path}: [{section}]: {key}= is invalid"
            if key in keys_invalid and keys_invalid[key] in valid:
                msg += f", did you mean {keys_invalid[key]}=?"

            logging.error(msg)
            exit_error_readme()

        verify_qemu_section(path, cfg, section)

        if section not in ["DEFAULT", "testsuite"] and "make" not in cfg[section]:
            logging.error(f"{path}: missing make= in section [{section}].")
            logging.error("If this is on purpose, set make=no.")
            exit_error_readme()

        for key in keys_lists:
            if key in cfg[section] and "  " in cfg[section][key]:
                logging.error(f"{path}: {key}= in section [{section}] has multiple spaces:")
                logging.error(f'  "{cfg[section][key]}"')
                logging.error("Please separate elements with only one space.")
                sys.exit(1)

    get_vty_host_port(cfg, path)


def raise_error_config_arg(glob_result, config_arg):
    valid = []
    for path in glob_result:
        basename = os.path.basename(path)
        if basename != "testenv.cfg":
            valid += [basename.split("_", 1)[1].split(".", -1)[0]]

    msg = f"Invalid parameter for --config: {config_arg}"

    if valid:
        msg += f" (valid: all, {', '.join(valid)})"
    else:
        msg += f" (the {testenv.args.testsuite} testsuite only has one testenv.cfg file, therefore just omit --config)"

    raise testenv.NoTraceException(msg)


def find_configs():
    dir_testsuite = os.path.join(testenv.testsuite.ttcn3_hacks_dir, testenv.args.testsuite)
    pattern = os.path.join(dir_testsuite, "testenv*.cfg")
    ret = sorted(glob.glob(pattern))

    if not ret:
        logging.error(f"Missing testenv.cfg in: {dir_testsuite}")
        exit_error_readme()
        sys.exit(1)

    if len(ret) > 1 and os.path.exists(os.path.join(dir_testsuite, "testenv.cfg")):
        logging.error("Found multiple testenv*.cfg, and a testenv.cfg.")
        logging.error("The testenv.cfg file must be renamed, consider naming it testenv_generic.cfg.")
        sys.exit(1)

    if len(ret) == 1 and not os.path.exists(os.path.join(dir_testsuite, "testenv.cfg")):
        logging.error("There is only one testenv*.cfg file, so please rename it:")
        logging.error(f"$ mv {os.path.basename(ret[0])} testenv.cfg")
        sys.exit(1)

    if len(ret) > 1 and not testenv.args.config:
        logging.error("Found multiple testenv.cfg files, use one of:")
        for path in ret:
            logging.error(f" -c {os.path.basename(path).replace('testenv_', '', 1).replace('.cfg', '')}")
        logging.error("You can also select all of them (-c all) or use the * character as wildcard.")
        sys.exit(1)

    return ret


def init():
    global cfgs

    cfgs_all = {}

    config_paths = find_configs()

    for path in config_paths:
        basename = os.path.basename(path)
        if basename != "testenv.cfg" and not basename.startswith("testenv_"):
            raise testenv.NoTraceException(
                f"Invalid filename, expected either testenv.cfg or testenv_*.cfg: {basename}"
            )
        if basename == "testenv_all.cfg":
            raise testenv.NoTraceException(f"Invalid filename: {basename}")

        cfg = configparser.ConfigParser()
        cfg.read(path)
        handle_latest(cfg, path)
        verify(cfg, path)

        # No --config argument given, and there is only one testenv.cfg
        if not testenv.args.config:
            cfgs[basename] = cfg
            verify_qemu_cfgs()
            return

        cfgs_all[basename] = cfg

    # Select configs based on --config argument(s)
    for config_arg in testenv.args.config:
        if config_arg == "all":
            if len(testenv.args.config) != 1:
                raise testenv.NoTraceException("Can't use multiple --config arguments if one of them is 'all'")
            cfgs = cfgs_all
            return

        matched = False
        for basename in cfgs_all:
            pattern = f"testenv_{config_arg}.cfg"
            if fnmatch.fnmatch(basename, pattern):
                matched = True
                cfgs[basename] = cfgs_all[basename]

        if not matched:
            raise_error_config_arg(config_paths, config_arg)

        verify_qemu_cfgs()
