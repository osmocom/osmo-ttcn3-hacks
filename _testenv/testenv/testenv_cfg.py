# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import configparser
import glob
import logging
import os.path
import sys
import testenv
import testenv.testsuite

cfgs = {}
current = None


def set_current(cfg_name):
    global current
    current = cfg_name

    if cfg_name == "testenv.cfg":
        testenv.set_log_prefix("[testenv]")
    else:
        cfg_name = cfg_name.replace("testenv_", "")
        cfg_name = cfg_name.replace(".cfg", "")
        testenv.set_log_prefix(f"[testenv][{cfg_name}]")


def exit_error_readme():
    readme = os.path.join(testenv.testsuite.ttcn3_hacks_dir_src, "_testenv/README.md")
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


def verify(cfg, path):
    keys_valid_testsuite = [
        "clean",
        "config",
        "copy",
        "program",
    ]
    keys_valid_component = [
        "clean",
        "copy",
        "make",
        "package",
        "prepare",
        "program",
        "setup",
        "vty_host",
        "vty_port",
    ]
    keys_invalid = {
        "configs": "config",
        "packages": "package",
        "programs": "program",
    }

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

        if section not in ["DEFAULT", "testsuite"] and "make" not in cfg[section]:
            logging.error(f"{path}: missing make= in section [{section}].")
            logging.error("If this is on purpose, set make=no.")
            exit_error_readme()

    get_vty_host_port(cfg, path)


def raise_error_config_arg(glob_result):
    valid = []
    for path in glob_result:
        basename = os.path.basename(path)
        if basename != "testenv.cfg":
            valid += [basename.split("_", 1)[1].split(".", -1)[0]]

    msg = f"Invalid parameter for --config: {testenv.args.config}"

    if valid:
        msg += f" (valid: all, {', '.join(valid)})"
    else:
        msg += f" (the {testenv.args.testsuite} testsuite only has one testenv.cfg file, therefore just omit --config)"

    raise testenv.NoTraceException(msg)


def find_configs():
    dir_testsuite = os.path.join(testenv.testsuite.ttcn3_hacks_dir_src, testenv.args.testsuite)
    pattern = os.path.join(dir_testsuite, "testenv*.cfg")
    ret = glob.glob(pattern)

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
        logging.error("Found multiple testenv.cfg files:")
        for path in ret:
            logging.error(f" * {os.path.basename(path)}")
        example = os.path.basename(ret[0]).split("_", 1)[1].split(".cfg", 1)[0]
        logging.error(f"Select a specific config (e.g. '-c {example}') or all ('-c all')")
        sys.exit(1)

    return ret


def init():
    global cfgs

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

        if not testenv.args.config:
            cfgs[basename] = cfg
            continue

        for config_arg in testenv.args.config:
            if config_arg == "all" or f"testenv_{config_arg}.cfg" == basename:
                cfgs[basename] = cfg
                break

    if not cfgs:
        raise_error_config_arg(config_paths)
