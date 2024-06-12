# testenv

Build everything needed for running Osmocom TTCN-3 testsuites and execute them.

## testenv.cfg

Each testsuite needs to have a `testenv.cfg`. One `[testsuite]` section must be
present, below that come one or more sections for test components that need to
be running to execute the tests.

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

These keys are valid in the `[testsuite]` section:

* `program=` the executable for starting the testsuite, without arguments.

* `config=`: the testsuite configuration file.

* `copy=`: additional file(s) to copy from the testsuite directory to the test
  directory, useful for include files mentioned in the config. Multiple values
  are separated by spaces.

#### Component section

* `program=`: executable for starting a test component, may contain arguments.
  `PATH` includes the test directory (see `copy=`), and
  `_testenv/data/scripts` (which contains e.g. `respawn.sh`).

* `copy=`: file(s) to copy from the testsuite directory to the test directory,
  like `.cfg` and `.confmerge` files and helper scripts for running the test
  components. Multiple values are separated by spaces.

* `make=`: osmo-dev make target for building from source, if running without
  `--binary-repo`. This is usually the name of the git repository, but could
  also be e.g. `.make.osmocom-bb.clone` to only clone and not build
  `osmocom-bb.git` for faketrx. Set `make=no` to not build/clone anything.

* `package=`: debian package(s) to be installed for running a test component
  and the script in `prepare=`. Multiple values separated by spaces. Set to
  `package=no` to not install any package.

* `prepare=`: optional script to run before staring the test component (after
  files are copied to the test directory). Typically this is used to create
  configs with `osmo-config-merge`. See `program=` for `PATH`.

* `setup=`: optional script to run after program was started. This can be used
  to wait until the program is ready or to fill a test database for example.
  See `program=` for `PATH`.

### Latest configs

Sometimes we need to run test components and/or testsuites with different
configurations depending on whether we are currently testing the nightly/master
versions of test components or the latest (stable) versions. For example, when
a new feature gets introduced that we need to configure and test, but which is
not available in the latest stable version.

For this purpose, it is possible to add configuration keys ending in `_latest`
to `testenv.cfg`. These keys will override the original keys if `testenv.py`
is running with `--latest` or with a binary repository ending in `:latest`.

It is also possible to take different code paths or exclude tests in the
TTCN-3 code if the latest configs are (not) used. This is done with
`f_osmo_repo_is` in `library/Misc_Helpers.ttcn`.
