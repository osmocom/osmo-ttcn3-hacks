import atexit
import datetime
import glob
import logging
import os.path
import tempfile
import testenv
import testenv.cmd
import testenv.testsuite
import os


# Some testsuites have multiple configurations (like bts: generic, hopping, …).
# For each configuration, prepare() gets called. testdir is the one for the
# current configuration.
testdir = None
testdir_topdir = None


def init():
    global testdir_topdir

    prefix = f"testenv-{testenv.args.testsuite}-"
    prefix += datetime.datetime.now().strftime("%Y%m%d-%H%M%S-")
    if testenv.args.log_dir:
        testdir_topdir = testenv.args.log_dir
        os.makedirs(testdir_topdir)
    else:
        testdir_topdir = tempfile.mkdtemp(prefix=prefix)

    atexit.register(clean)

    logging.info(f"Logging to: {testdir_topdir}")

    # Add a convenience symlink
    if not testenv.args.log_dir:
        if os.path.exists("/tmp/logs"):
            testenv.cmd.run(["rm", "/tmp/logs"], no_podman=True)
        testenv.cmd.run(["ln", "-sf", testdir_topdir, "/tmp/logs"],
                        no_podman=True)


def prepare(cfg_name, cfg):
    global testdir

    if len(testenv.testenv_cfg.cfgs) == 1:
        testdir = testdir_topdir
    else:
        testdir = os.path.join(testdir_topdir, cfg_name)

    logging.info(f"Preparing testdir: {testdir}")
    testsuite_dir = os.path.join(testenv.testsuite.ttcn3_hacks_dir,
                                 testenv.args.testsuite)

    for section in cfg:
        if section in ["DEFAULT"]:
            continue

        section_data = cfg[section]
        section_dir = os.path.join(testdir, section)
        os.makedirs(section_dir)

        if "config" in section_data and section == "testsuite":
            file = section_data["config"]
            path = os.path.join(testsuite_dir, file)
            path_dest = os.path.join(section_dir, file)
            testenv.cmd.run(["install", "-Dm644", path, path_dest])

        if "copy" in section_data:
            for file in section_data["copy"].split(" "):
                path = os.path.join(testsuite_dir, file)
                path_dest = os.path.join(section_dir, file)
                testenv.cmd.run(["install", "-Dm644", path, path_dest])

        if "prepare" in section_data:
            testenv.cmd.run(section_data["prepare"], cwd=section_dir)


    # Referenced in testsuite cfgs: *.default
    pattern = os.path.join(testsuite_dir, "*.default")
    for path in glob.glob(pattern):
        path_dest = os.path.join(testdir, "testsuite",
                                 os.path.basename(path))
        testenv.cmd.run(["install", "-Dm644", path, path_dest])

    # Referenced in testsuite cfgs: ../Common.cfg
    common_cfg = os.path.join(testdir, "Common.cfg")
    path = os.path.join(testenv.testsuite.ttcn3_hacks_dir, "Common.cfg")
    testenv.cmd.run(["install", "-Dm644", path, common_cfg])
    testenv.cmd.run(["sed", "-i",
                     f"s#TTCN3_HACKS_PATH := .*#TTCN3_HACKS_PATH := \"{testenv.testsuite.ttcn3_hacks_dir}\"#",
                     common_cfg])

    # Set mp_osmo_repo in the testsuite config
    mp_osmo_repo = "latest" if testenv.args.latest else "nightly"
    line = f"Misc_Helpers.mp_osmo_repo := \"{mp_osmo_repo}\""
    testsuite_cfg = os.path.join(testdir, "testsuite", cfg["testsuite"]["config"])
    testenv.cmd.run(["sed", "-i",
                     f"s/\[MODULE_PARAMETERS\]/\[MODULE_PARAMETERS\]\\n{line}/g",
                     testsuite_cfg])


def clean():
    """Don't leave behind an empty testdir_topdir, e.g. if testenv.py aborted
       during build of components."""
    if os.listdir(testdir_topdir):
        # Dir is not empty, keep it
        msg = f"Logs saved to: {testdir_topdir}"
        if not testenv.args.log_dir:
            msg += " (symlink: /tmp/logs)"
        logging.info(msg)
        return

    logging.debug("Remving empty log dir")
    testenv.cmd.run(["rm", "-d", testdir_topdir], no_podman=True)

    # Remove broken symlink
    if not testenv.args.log_dir and os.path.lexists("/tmp/logs") \
            and not os.path.exists("/tmp/logs"):
        testenv.cmd.run(["rm", "/tmp/logs"], no_podman=True)
