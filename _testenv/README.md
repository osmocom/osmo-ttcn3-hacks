# testenv

Testenv builds and runs a testsuite and all the components it tests (usually
one or more Osmocom programs). See `./testenv.py -h` for command-line usage
help.

Example for running the `mgw` testsuite:
```
$ ./testenv.py run mgw
```

## Virtualization

By default testenv does everything directly on the host, without using a
container. The `--podman` parameter can be used to build and run everything in
a container instead.

When passing `--podman`, then just one container is used to build and run
everything. The container is the same, no matter which testsuite gets executed.
Additional packages get installed after starting the container, with a package
cache mounted to avoid unnecessary downloads.

### QEMU

For SUTs that interact with kernel drivers, it is desirable to run them with a
separate kernel in QEMU. This can be enabled by running `./testenv.py run` with
the `-D` (Debian kernel) or `-C` (custom kernel) arguments. See the `ggsn`
testsuite for reference.

#### Custom kernel

When using `-C`, testenv uses a `.linux` file in the `osmo-ttcn3-hacks` dir as
kernel. You can download a pre-built kernel from jenkins:

```
$ wget -O .linux "https://jenkins.osmocom.org/jenkins/job/build-kernel-torvalds/lastSuccessfulBuild/artifact/output/linux"
$ wget -O .linux "https://jenkins.osmocom.org/jenkins/job/build-kernel-net-next/lastSuccessfulBuild/artifact/output/linux"
```

Or build your own kernel, see:
https://gitea.osmocom.org/osmocom/osmo-ci/src/branch/master/scripts/kernel

## testenv.cfg

The `testenv.cfg` file has one `[testsuite]` section, typically with one or
more sections for test components.

### Example

```ini
[testsuite]
program=MGCP_Test
config=MGCP_Test.cfg

[mgw]
program=osmo-mgw
make=osmo-mgw
package=osmo-mgw
copy=osmo-mgw.cfg
vty_port=4243
```

### Keys

#### Testsuite section

* `program=` the executable for starting the testsuite, without arguments.

* `config=`: the testsuite configuration file.

* `copy=`: additional file(s) to copy from the testsuite directory to the test
  directory, useful for include files mentioned in the config. Multiple values
  are separated by spaces.

* `prepare=`: optional script to run before staring the testsuite (after files
  are copied to the test directory). This can be used to change module
  parameters in the testsuite's config.

* `clean=`: optional script to run before running the testsuite and on exit.
  This can be used to clean up network devices for example, or to fix name
  collisions when running a test with multiple configs
  (`rename_junit_xml_classname.sh`). See below for `PATH` and `PWD`. A
  `TESTENV_CLEAN_REASON` env var is set to `prepare`, `crashed` or `finished`
  depending on when the script runs. The script will not run on crash if podman
  is used, as the container gets shutdown beforehand.

#### Component section

* `program=`: executable for starting a test component, may contain arguments.
  See below for `PATH` and `PWD`.

* `copy=`: optionally copy a space separated list of files or directories from
  the testsuite directory to the test directory, like `.cfg` and `.confmerge`
  files. Testenv runs `cp -a <SOURCES> <TEST DIR>` internally.

* `make=`: osmo-dev make target for building from source, if running without
  `--binary-repo`. This is usually the name of the git repository, but could
  also be e.g. `.make.osmocom-bb.clone` to only clone and not build
  `osmocom-bb.git` for faketrx. Set `make=no` to not build/clone anything.

* `package=`: debian package(s) to be installed for running a test component
  and the script in `prepare=`. Multiple values separated by spaces. Set to
  `package=no` to not install any package.

* `prepare=`: optional script to run before staring the program (after files
  are copied to the test directory). Typically this is used to create configs
  with `osmo-config-merge`. See below for `PATH` and `PWD`.

* `setup=`: optional script to run after the program was started. Execution of
  the next program / the testsuite will wait until the setup script has quit.
  This can be used to wait until the program is ready or to fill a test
  database for example. See below for `PATH` and `PWD`.

* `clean=`: same as in the Testsuite section above, but runs at a slightly
  different time: before/after/on crash of the program instead of the
  testsuite.

* `vty_port=`: optionally set the VTY port for the SUT component to obtain a
  talloc report after each test case has been executed. If this is not set, no
  talloc reports will be obtained.

* `vty_host=`: optionally set the VTY host for the SUT component to be used
  when obtaining a talloc report. If this is not set, `127.0.0.1` is used.

* `qemu=`: set to `optional`/`required` to allow/require running this test
  component in QEMU. Additional logic must be added to build an initrd with the
  test component and actually run it in QEMU, this is done by sourcing
  `qemu_functions.sh` and using functions from there. See the `ggsn` testsuite
  for reference.

### Latest configs

Sometimes we need to run test components and/or testsuites with different
configurations depending on whether we are currently testing the nightly/master
versions of test components or the latest (stable) versions. For example, when
a new feature gets introduced that we need to configure and test, but which is
not available in the latest stable version.

For this purpose, it is possible to add configuration keys ending in `_latest`
to `testenv.cfg`. These keys will override the original keys if `testenv.py`
is running with a binary repository ending in `:latest` or with `--latest`.

It is also possible to take different code paths or exclude tests in the
TTCN-3 code if the latest configs are (not) used. This is done with
`f_osmo_repo_is` in `library/Misc_Helpers.ttcn`.

### Multiple testenv.cfg files

Usually each testsuite has only one `testenv.cfg` file, e.g. `mgw/testenv.cfg`.
For some testsuites it is necessary to run them multiple times with slightly
different configurations. In that case, we have multiple `testenv.cfg` files,
typically one `testenv_generic.cfg` and additional `testenv_<NAME>.cfg` files.

For example:
* `bts/testenv_generic.cfg`
* `bts/testenv_hopping.cfg`
* `bts/testenv_oml.cfg`

## Environment variables

### Environment variables read by testenv

You can set the following environment variables to change testenv behaviour.

* `TESTENV_SRC_DIR`:
  Set the directory for sources of Osmocom components. The default is the
  directory above your osmo-ttcn3-hacks.git clone.

* `TESTENV_REBUILD_OUTDATED_IMAGE`:
  Automatically rebuild the outdated image, instead of only displaying a
  warning. This is not the default because rebuilding it takes some time and is
  oftentimes not needed (e.g. if a dependency was added that is not relevant to
  the testsuite that the user is currently testing).

* `TESTENV_NO_IMAGE_UP_TO_DATE_CHECK`:
  Do not compare the timestamp of `data/podman/Dockerfile` with the date of the
  podman image. This check does not work on jenkins where we always have
  a fresh clone.

* `TESTENV_COLOR_{DEBUG|INFO|WARNING|ERROR|CRITICAL|RESET}`:
  Change the colors for different log levels (we use this in Jenkins, which
  prints the output on white background). Find the defaults in
  `testenv/__init__.py:ColorFormatter()`.

* `TESTENV_SOURCE_HIGHLIGHT_COLORS`:
  The argument to pass to `source-highlight` for formatting the junit log xml
  files. Jenkins sets this to `esc` instead of `esc256` for better contrast on
  white background.

* `TESTENV_NO_KVM`:
  Do not mount /dev/kvm in podman. This is used in jenkins where /dev/kvm is
  available but doesn't work in podman. QEMU runs a bit slower when this is
  set.

* `TESTENV_COREDUMP_FROM_LXC_HOST`:
  Instead of using coredumpctl to retrieve the coredump, assume testenv is
  running inside an LXC and try to retrieve the coredump from the LXC host
  (OS#6769). This is used in jenkins.

* `TESTENV_COREDUMP_FROM_LXC_HOST_IP`:
  Instead of attempting to automatically detect the LXC host IP, use this IP.
  This can be set to 127.0.0.1 for testing.

### Environment variables set by testenv

Testenv sets the following variables while running shell commands from
`program=`, `prepare=`, `setup=` and `clean=`. The variables are set in
`testenv/cmd.py:init_env()`.

* `PATH`:
  The user's `PATH` environment variable is prefixed with the following paths:
  * The directory of the testsuite.
  * The directory for binaries built from source.
  * The directory `_testenv/data/scripts` (which has e.g. `respawn.sh`).
  * The directory `_testenv/data/scripts/qemu`.

* `PWD`:
  The executables run inside a directory with the component name, inside the
  log dir. For example:

```
/tmp/logs
├── ggsn                   # Executables from [ggsn] section run in this dir
│   ├── ggsn.log
│   └── osmo-ggsn.cfg
└── testsuite              # Executables from [testsuite] run in this dir
    ├── Common.cfg
    ├── GGSN_Tests.cfg
    └── GGSN_Tests.default
```

* `TESTENV_INSTALL_DIR`:
  The directory into which the SUT binaries and other files are installed. It
  is set to `/` for `--podman --binary-repo`,
  `~/.cache/osmo-ttcn3-testenv/podman/install` for `--podman` and
  `~/.cache/osmo-ttcn3-testenv/host/install` otherwise. The
  `~/.cache/osmo-ttcn3-testenv` part can be changed with the `--cache`
  argument. Different cache directories for podman and for the host are used
  as it is very likely that the binary objects from both are incompatible.

* `TESTENV_SRC_DIR`:
  Is set to the directory for sources of Osmocom components. The default is the
  directory above the osmo-ttcn3-hacks.git clone. This can be changed by the
  user by having `TESTENV_USR_DIR` set while running `testenv.py`.

* `OSMO_DEV_MAKE_DIR`:
  This variable is unset if `--binary-repo` is used as argument. Otherwise it
  is set to `~/.cache/osmo-ttcn3-testenv/host/make<version>` or
  `~/.cache/osmo-ttcn3-testenv/podman/make<version>` (with `--podman`) by
  default. The `~/.cache/osmo-ttcn3-testenv` part can be changed with the
  `--cache` argument.

* `TESTENV_QEMU_KERNEL`:
  Is only set if `-C`/`--custom-kernel` or `-D`/`--debian-kernel` parameters
  are set. With `-C` it is set to `<path to osmo-ttcn3-hacks>/.linux`. With
  `-D` it is set to `debian`.

* `TESTENV_QEMU_SCRIPTS`:
  Is set to `<path to osmo-ttcn3-hacks>/_testenv/data/scripts/qemu`.

* `CCACHE_DIR`:
  Is set to the value of the `--ccache` parameter, which is
  `~/.cache/osmo-ttcn3-testenv/ccache` by default.

* `PKG_CONFIG_PATH`:
  Is prefixed with `$TESTENV_INSTALL_DIR/usr/lib/pkgconfig:`.

* `LD_LIBRARY_PATH`:
  Is prefixed with `$TESTENV_INSTALL_DIR/usr/lib:`.

* `TERM`:
  Is set to the same `TERM` passed to testenv with fallback to `dumb`.

## Troubleshooting

### Timeout waiting for RESET-ACK after sending RESET

This can happen if another Osmocom program is started before OsmoSTP, then
tries to connect to OsmoSTP and fails, and waits several seconds before
retrying. Check the logs to confirm this is the case. To fix this, adjust your
`testenv.cfg` to start OsmoSTP before other Osmocom programs. The testenv
scripts will wait a bit to give OsmoSTP enough time to start up, before
starting the other test components.
