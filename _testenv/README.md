# testenv

Build everything needed for running Osmocom TTCN-3 testsuites and execute them.

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
```

### Keys

#### Testsuite section

* `program=` the executable for starting the testsuite, without arguments.

* `config=`: the testsuite configuration file.

* `copy=`: additional file(s) to copy from the testsuite directory to the test
  directory, useful for include files mentioned in the config. Multiple values
  are separated by spaces.

* `clean=`: optional script to run before running the testsuite and on exit.
  This can be used to clean up network devices for example, or to fix name
  collisions when running a test with multiple configs
  (`rename_junit_xml_classname.sh`). See below for `PATH`. A
  `TESTENV_CLEAN_REASON` env var is set to `prepare`, `crashed` or `finished`
  depending on when the script runs.

#### Component section

* `program=`: executable for starting a test component, may contain arguments.
  See below for `PATH`.

* `copy=`: file(s) to copy from the testsuite directory to the test directory,
  like `.cfg` and `.confmerge` files. Multiple values are separated by spaces.

* `make=`: osmo-dev make target for building from source, if running without
  `--binary-repo`. This is usually the name of the git repository, but could
  also be e.g. `.make.osmocom-bb.clone` to only clone and not build
  `osmocom-bb.git` for faketrx. Set `make=no` to not build/clone anything.

* `package=`: debian package(s) to be installed for running a test component
  and the script in `prepare=`. Multiple values separated by spaces. Set to
  `package=no` to not install any package.

* `prepare=`: optional script to run before staring the program (after files
  are copied to the test directory). Typically this is used to create configs
  with `osmo-config-merge`. See below for `PATH`.

* `setup=`: optional script to run after the program was started. Execution of
  the next program / the testsuite will wait until the setup script has quit.
  This can be used to wait until the program is ready or to fill a test
  database for example. See below for `PATH`.

* `clean=`: optional script to run before `prepare=` and on exit. This can be
  used to clean up network devices for example, or to fix name collisions when
  running a test with multiple configs (`rename_junit_xml_classname.sh`). See
  below for `PATH`. A `TESTENV_CLEAN_REASON` env var is set to `prepare`,
  `crashed` or `finished` depending on when the script runs.

### PATH

Executables mentioned in `program=`, `prepare=`, `setup=` and `clean=` run
with a `PATH` environment variable containing:

* The directory of the testsuite
* The directory for binaries built from source
* The directory `_testenv/data/scripts` (which has e.g. `respawn.sh`)

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

* `TESTENV_SRC_DIR`:
  Set the directory for sources of Osmocom components. The default is the
  directory above your osmo-ttcn3-hacks.git clone.

* `TESTENV_NO_IMAGE_UP_TO_DATE_CHECK`:
  Do not compare the timestamp of `data/virt/Dockerfile` with the date of the
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

## Troubleshooting

### Timeout waiting for RESET-ACK after sending RESET

This can happen if another Osmocom program is started before OsmoSTP, then
tries to connect to OsmoSTP and fails, and waits several seconds before
retrying. Check the logs to confirm this is the case. To fix this, adjust your
`testenv.cfg` to start OsmoSTP before other Osmocom programs. The testenv
scripts will wait a bit to give OsmoSTP enough time to start up, before
starting the other test components.
