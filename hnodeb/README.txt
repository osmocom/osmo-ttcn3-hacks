Integration Tests for OsmoHnodeB
--------------------------------

This test suite tests OsmoHNodeB while emulating both multiple UE as
well as the HNBGW.

The included jenkins.sh script, together with the Dockerfiles from
http://git.osmocom.org/docker-playground/ can be used to run both the
osmo-hnodeb-under-test as well as the extenal entities and the tester.


Further Test Ideas
------------------

This is a random list of things about things possible to test.
Asterisks '*' are TODO, while 'x' means already implemented.

= exhaustion of resources

= paging

= assignment

= hand-over

= erroneous channel release

= misc

= counters

= VTY based/corresponding tests
