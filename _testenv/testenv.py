#!/usr/bin/env python3
# Copyright 2024 sysmocom - s.f.m.c. GmbH
# SPDX-License-Identifier: GPL-3.0-or-later
import logging
import os
import sys
import testenv
import testenv.cmd
import testenv.daemons
import testenv.osmo_dev
import testenv.podman
import testenv.podman_install
import testenv.requirements
import testenv.testdir
import testenv.testenv_cfg
import testenv.testsuite


def loop_continue_cond(loop_count):
    if loop_count == 0:
        return True

    if testenv.args.until_nok:
        logging.info("Checking testsuite logs for failures and errors")
        for match_str in ["failures='0'", "errors='0'"]:
            if not testenv.testsuite.check_junit_logs_have(loop_count - 1, match_str):
                logging.critical("Stopping the loop")
                return False
        return True
    else:
        return False


def run():
    testenv.testenv_cfg.init()

    if not testenv.args.binary_repo:
        testenv.osmo_dev.check_init_needed()

    testenv.requirements.check()
    testenv.requirements.mount_sys_fs_bpf()
    testenv.podman_install.init()
    testenv.cmd.init_env()
    testenv.testdir.init()
    testenv.daemons.init()

    if testenv.args.podman:
        testenv.podman.init()
        testenv.podman.start(testenv.testenv_cfg.get_first())

    if not testenv.args.binary_repo:
        testenv.osmo_dev.init()

    testenv.testsuite.init()
    testenv.testsuite.build()

    # Build all components first
    if not testenv.args.binary_repo:
        for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
            testenv.testenv_cfg.set_current(cfg_name)
            testenv.osmo_dev.make(cfg)

    # Run the components + testsuite
    loop_count = 0
    while loop_continue_cond(loop_count):
        # Restart podman container before running again
        if testenv.args.podman and loop_count:
            testenv.podman.stop(testenv.testenv_cfg.get_first())

        cfg_count = 0
        for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
            # Restart podman container before running with another config
            if testenv.args.podman and cfg_count:
                testenv.podman.stop(cfg)

            testenv.testenv_cfg.set_current(cfg_name, loop_count)

            if testenv.args.binary_repo:
                testenv.podman.enable_binary_repo()
                testenv.podman_install.packages(cfg, cfg_name)

            testenv.testdir.prepare(cfg_name, cfg, loop_count)
            testenv.daemons.start(cfg)
            testenv.testsuite.run(cfg)
            testenv.daemons.stop()
            testenv.testdir.clean_run_scripts("finished")

            cfg_count += 1
            testenv.set_log_prefix("[testenv]")

        loop_count += 1

    # Show test results
    testenv.testsuite.cat_junit_logs()

    if testenv.args.podman:
        testenv.podman.stop()


def init_podman():
    testenv.podman.init_image_name_distro()
    testenv.podman.image_build()


def init_osmo_dev():
    testenv.osmo_dev.init_clone()


def clean():
    cache_dirs = [
        "git",
        "host",
        "podman",
    ]
    for cache_dir in cache_dirs:
        path = os.path.join(testenv.cache_dir_default, cache_dir)
        if os.path.exists(path):
            testenv.cmd.run(["rm", "-rf", path])


def main():
    testenv.init_logging()
    testenv.init_args()

    action = testenv.args.action
    if action == "run":
        run()
    elif action == "init":
        if testenv.args.runtime == "osmo-dev":
            init_osmo_dev()
        elif testenv.args.runtime == "podman":
            init_podman()
    elif action == "clean":
        clean()


try:
    main()
except testenv.NoTraceException as e:
    logging.error(e)
    testenv.podman.stop()
    sys.exit(2)
except KeyboardInterrupt:
    print("")  # new line
    test = testenv.testsuite.get_current_test()
    if test:
        logging.critical(f"^C during {test}")
    testenv.podman.stop()
    sys.exit(3)
except:
    testenv.podman.stop()
    raise
