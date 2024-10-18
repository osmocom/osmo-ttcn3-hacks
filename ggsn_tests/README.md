# GGSN_Tests.ttcn

## External interfaces

* Gp: GTP (emulates SGSN)
* Gi: IP (emulates Internet)
* VTY

{% dot ggsn_tests.svg
digraph G {
  rankdir=LR;
  GGSN [label="GGSN\nosmo-ggsn",shape="box"];
  ATS [label="ATS\nGGSN_Tests.ttcn"];

  ATS -> GGSN [label="Gp (GTP)"];
  GGSN -> ATS [label="Gi (IP)"];
  ATS -> GGSN [label="VTY"];
}
%}

## How to run

### osmo-ggsn

osmo-ggsn with APN config [all](osmo-ggsn/osmo-ggsn-all.confmerge):
```
$ ./testenv.py run ggsn -c osmo_ggsn_all
```

osmo-ggsn with APN config [v4_only](osmo-ggsn/osmo-ggsn-v4_only.confmerge):
```
$ ./testenv.py run ggsn -c osmo_ggsn_v4_only
```

osmo-ggsn with APN configs
[all](osmo-ggsn/osmo-ggsn-all.confmerge),
[v4_only](osmo-ggsn/osmo-ggsn-v4_only.confmerge),
[v6_only](osmo-ggsn/osmo-ggsn-v6_only.confmerge) and
[v4v6_only](osmo-ggsn/osmo-ggsn-v4v6_only.confmerge):
```
$ ./testenv.py run ggsn -c 'osmo_ggsn_*'
```

### osmo-ggsn + kernel GTP-U

osmo-ggsn with APN config [v4_only](osmo-ggsn/osmo-ggsn-v4_only.confmerge)
and kernel GTP-U with a custom kernel:
```
$ wget -O .linux https://jenkins.osmocom.org/jenkins/job/build-kernel-net-next/lastSuccessfulBuild/artifact/output/linux
$ ./testenv.py run ggsn -c osmo_ggsn_v4_only -C
```

osmo-ggsn with APN config [v4_only](osmo-ggsn/osmo-ggsn-v4_only.confmerge)
and kernel GTP-U with Debian kernel:
```
$ ./testenv.py run ggsn -c osmo_ggsn_v4_only --podman -D
```

osmo-ggsn with APN configs
[v4_only](osmo-ggsn/osmo-ggsn-v4_only.confmerge),
[v6_only](osmo-ggsn/osmo-ggsn-v6_only.confmerge) and
[v4v6_only](osmo-ggsn/osmo-ggsn-v4v6_only.confmerge)
and kernel GTP-U with Debian kernel:
```
$ ./testenv.py run ggsn -c 'osmo_ggsn_v*_only' --podman -D
```

### open5gs

```
$ ./testenv.py run ggsn -c open5gs
```

## Related Jenkins jobs

The jenkins jobs have the testenv arguments they use in their description.

* [ttcn3-ggsn-test](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test/)
* [ttcn3-ggsn-test-latest](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-latest/)
* [ttcn3-ggsn-test-kernel](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel/)
* [ttcn3-ggsn-test-kernel-latest](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-latest/)
* [ttcn3-ggsn-test-kernel-net-next](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-net-next/)
* [ttcn3-ggsn-test-kernel-latest-net-next](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-latest-net-next/)
* [ttcn3-ggsn-test-kernel-torvalds](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-torvalds/)
* [ttcn3-ggsn-test-kernel-latest-torvalds](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-latest-torvalds/)
* [ttcn3-ggsn-test-ogs](https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-ogs/)
