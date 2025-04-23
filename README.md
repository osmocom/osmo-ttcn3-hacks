# Osmocom TTCN-3 Test Suites

This repository contains a collection of test suites developed within
the [Osmocom](https://osmocom.org/) (Open Source Mobile Communications)
project.  The test suites are developed in the
[TTCN-3](https://de.wikipedia.org/wiki/TTCN-3) programming language,
compiled/executed by the [Eclipse TITAN](https://projects.eclipse.org/projects/tools.titan)
compiler and runtime.

Those test suites mostly are performing *functional testing* of cellular
network elements, from 2G, 3G, 4G to 5G.  The individual test-suites are
in sub-directories, while some shared library code is in *library*.

## Running Testsuites

Use the `testenv.py` script to run the testsuites, e.g.:

```
$ ./testenv.py run mgw
```

This will build the mgw testsuite from source, as well as osmo-mgw and all its
dependencies and then run the testsuite against it. The Osmocom sources will be
cloned if they don't exist in a sibling directory to osmo-ttcn3-hacks. It is
also possible to use the Osmocom binary packages with this and running
everything inside a podman container. See `./testenv.py -h` and
`_testenv/README.md` for more information.

### Manually

Alternatively you can build and run the testsuite yourself, e.g.:

```
$ make mgw
$ cd mgw
$ ../start-testsuite.sh ../_build/mgw/MGCP_Test MGCP_Test.cfg
```

Make sure that all components that the testsuite requires are running
(`osmo-mgw` in this example).

## Continuous Integration

The individual tests suites are executed against different versions of
the respective IUT (Implementation Under Test) by the Osmocom jenkins.

See the [list of all TTCN-3 jenkins jobs](https://jenkins.osmocom.org/jenkins/view/TTCN3/)
for more details

## Further reading

Some more information about those test suites can be found
at <https://osmocom.org/projects/cellular-infrastructure/wiki/Titan_TTCN3_Testsuites>.
