#!/usr/bin/env python3
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


def run():
    testenv.testenv_cfg.init()
    testenv.requirements.check()
    testenv.podman_install.init()
    testenv.cmd.init_env()
    testenv.testdir.init()
    testenv.daemons.init()

    if testenv.args.podman:
        testenv.podman.init()
        testenv.podman.start()

    if not testenv.args.binary_repo:
        testenv.osmo_dev.init()

    testenv.testsuite.init()
    testenv.testsuite.build()

    # Run prepare functions of testsuites (may enable extra repos)
    for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
        testenv.testsuite.run_prepare_script(cfg)

    if testenv.args.binary_repo:
        testenv.podman.enable_binary_repo()

    # Build/install all components first
    for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
        if testenv.args.binary_repo:
            testenv.podman_install.packages(cfg, cfg_name)
        else:
            testenv.osmo_dev.make(cfg)

    # Run the components + testsuite
    for cfg_name, cfg in testenv.testenv_cfg.cfgs.items():
        testenv.testdir.prepare(cfg_name, cfg)
        testenv.daemons.start(cfg)
        testenv.testsuite.run(cfg)

def image():
    testenv.podman.init_image_name_distro()
    testenv.podman.image_build()


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
    elif action == "image":
        image()
    elif action == "clean":
        clean()


try:
    main()
except testenv.NoTraceException as e:
    logging.error(e)
    sys.exit(2)
except KeyboardInterrupt:
    print("")  # new line
    sys.exit(3)
