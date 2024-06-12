import logging
import multiprocessing
import os.path
import shlex
import testenv
import testenv.cmd

ttcn3_hacks_dir = None
ttcn3_hacks_dir_src = os.path.realpath(f"{__file__}/../../..")


def update_deps():
    deps_marker = os.path.join(testenv.args.cache, "ttcn3-deps-updated")
    if os.path.exists(deps_marker):
        return

    logging.info("Updating osmo-ttcn3-hacks/deps")
    deps_dir = os.path.join(ttcn3_hacks_dir_src, "deps")
    testenv.cmd.run(["make", "-C", deps_dir])
    testenv.cmd.run(["touch", deps_marker])


def copy_ttcn3_hacks_dir():
    """Copy source files of osmo-ttcn3-hacks.git to the cache dir, so we don't
       mix binary objects from host and inside podman that are very likely to
       be incompatible"""
    global ttcn3_hacks_dir

    ttcn3_hacks_dir = os.path.join(testenv.args.cache,
                                   "podman",
                                   "osmo-ttcn3-hacks")

    logging.info(f"Copying osmo-ttcn3-hacks sources to: {ttcn3_hacks_dir}")

    # Rsync can't directly parse the .gitignore with ! rules, so create a list
    # of files to be excluded first
    exclude_list = os.path.join(testenv.args.cache, "podman",
                                "rsync-exclude")
    testenv.cmd.run("git"
                    " ls-files"
                    " --exclude-standard"
                    " -oi"
                    " --directory"
                    f" > {shlex.quote(exclude_list)}",
                    cwd=ttcn3_hacks_dir_src)

    # Copy source files, excluding binary objects
    testenv.cmd.run(["rsync",
                     "-a",
                     "--exclude", "/.git",
                     "--exclude-from", exclude_list,
                     f"{ttcn3_hacks_dir_src}/",
                     f"{ttcn3_hacks_dir}/"])

    # The "deps" dir is in gitignore, copy it separately
    testenv.cmd.run(["rsync",
                     "-a",
                     "--exclude", "/.git",
                     f"{ttcn3_hacks_dir_src}/deps/",
                     f"{ttcn3_hacks_dir}/deps/"])


def prepare_testsuite_dir():
    testsuite_dir = f"{ttcn3_hacks_dir}/{testenv.args.testsuite}"
    logging.info(f"Generating links and Makefile for {testenv.args.testsuite}")
    testenv.cmd.run(["./gen_links.sh"], cwd=testsuite_dir)
    testenv.cmd.run("USE_CCACHE=1 ./regen_makefile.sh", cwd=testsuite_dir)


def init():
    global ttcn3_hacks_dir

    update_deps()

    if testenv.args.podman:
        copy_ttcn3_hacks_dir()
    else:
        ttcn3_hacks_dir = ttcn3_hacks_dir_src

    prepare_testsuite_dir()


def build():
    logging.info(f"Building testsuite {testenv.args.testsuite}")
    testsuite_dir = f"{ttcn3_hacks_dir}/{testenv.args.testsuite}"
    testenv.cmd.run(["make", "compile"], cwd=testsuite_dir)

    jobs = multiprocessing.cpu_count() + 1
    testenv.cmd.run(["make", "-j", f"{jobs}"], cwd=testsuite_dir)


def run(cfg):
    section_data = cfg["testsuite"]

    cwd = os.path.join(testenv.testdir.testdir, "testsuite")
    start_testsuite = os.path.join(ttcn3_hacks_dir, "start-testsuite.sh")
    suite = os.path.join(ttcn3_hacks_dir, testenv.args.testsuite,
                         section_data["program"])

    env = {
        "TTCN3_PCAP_PATH": os.path.join(testenv.testdir.testdir, "testsuite"),
    }

    cmd = [start_testsuite, suite, section_data["config"]]

    test_arg = testenv.args.test
    if test_arg:
        if "." in test_arg:
            cmd += [test_arg]
        else:
            cmd += [f"{section_data['program']}.{test_arg}"]

    logging.info("Starting testsuite")

    testenv.cmd.run(cmd, cwd=cwd, env=env)

    logging.info("Merging log files")
    log_merge = os.path.join(ttcn3_hacks_dir, "log_merge.sh")
    cmd = [log_merge, section_data["program"], "--rm"]
    testenv.cmd.run(cmd, cwd=cwd)


def run_prepare_script(cfg):
    section_data = cfg["testsuite"]
    if "prepare" not in section_data:
        return

    logging.info("Running [testsuite] prepare script")
    testenv.cmd.run(section_data["prepare"])
