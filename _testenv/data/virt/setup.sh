#!/bin/sh -ex
export DEBIAN_FRONTEND=noninteractive

# Install packages from Debian repositories (alphabetic order)
apt-get update
apt-get install -y --no-install-recommends \
	autoconf \
	automake \
	bison \
	build-essential \
	ca-certificates \
	ccache \
	cmake \
	flex \
	git \
	iproute2 \
	iputils-ping \
	libbson-dev \
	libc-ares-dev \
	libcsv-dev \
	libcurl4-gnutls-dev \
	libdbd-sqlite3 \
	libdbi-dev \
	libgcrypt-dev \
	libgnutls28-dev \
	libidn11-dev \
	libjansson-dev \
	libmicrohttpd-dev \
	libmnl-dev \
	libmongoc-dev \
	libnghttp2-dev \
	libortp-dev \
	libpcap-dev \
	libpcsclite-dev \
	libsctp-dev \
	libsofia-sip-ua-glib-dev \
	libsqlite3-dev \
	libssl-dev \
	libtalloc-dev \
	libtins-dev \
	libtool \
	libulfius-dev \
	liburing-dev \
	libusb-1.0-0-dev \
	libyaml-dev \
	libzmq3-dev \
	meson \
	netcat-openbsd \
	pcscd \
	pkg-config \
	procps \
	psmisc \
	python3-pip \
	rebar3 \
	rsync \
	source-highlight \
	sqlite3 \
	sudo \
	tcpdump \
	vim \
	wget \
	wireshark-common
apt-get clean

# Binary-only transcoding library for RANAP/RUA/HNBAP to work around TITAN only implementing BER
DPKG_ARCH="$(dpkg --print-architecture)"
wget https://ftp.osmocom.org/binaries/libfftranscode/libfftranscode0_0.5_${DPKG_ARCH}.deb
wget https://ftp.osmocom.org/binaries/libfftranscode/libfftranscode-dev_0.5_${DPKG_ARCH}.deb
dpkg -i ./libfftranscode0_0.5_${DPKG_ARCH}.deb ./libfftranscode-dev_0.5_${DPKG_ARCH}.deb
apt-get install --fix-broken
rm libfftranscode*.deb

# Install osmo-python-tests (for obtaining talloc reports from SUT)
git clone --depth=1 https://gerrit.osmocom.org/python/osmo-python-tests osmo-python-tests
pip3 install ./osmo-python-tests --break-system-packages
rm -rf osmo-python-tests

# Add eclipse-titan from osmocom:latest
OSMOCOM_REPO_TESTSUITE_MIRROR="https://downloads.osmocom.org"
OSMOCOM_REPO="$OSMOCOM_REPO_TESTSUITE_MIRROR/packages/osmocom:/latest/Debian_12/"
echo "deb [signed-by=/obs.key] $OSMOCOM_REPO ./" > /etc/apt/sources.list.d/osmocom-latest.list
apt-get update
apt-get install -y --no-install-recommends eclipse-titan
apt-get clean
rm /etc/apt/sources.list.d/osmocom-latest.list

# Add mongodb for open5gs-hss. Using the package from bullseye since bookworm
# mongodb-org package is not available. Furthermore, manually install required
# libssl1.1.
mkdir -p /tmp/mongodb
cd /tmp/mongodb
wget "https://pgp.mongodb.com/server-5.0.asc" -O "/mongodb.key"
wget "http://security.debian.org/debian-security/pool/updates/main/o/openssl/libssl1.1_1.1.1n-0+deb10u6_amd64.deb"
dpkg -i "libssl1.1_1.1.1n-0+deb10u6_amd64.deb"
echo "deb [signed-by=/mongodb.key] http://repo.mongodb.org/apt/debian bullseye/mongodb-org/5.0 main" \
	> /etc/apt/sources.list.d/mongodb-org.list
apt-get update
apt-get install -y mongodb-org
apt-get clean
cd /
rm -rf /tmp/mongodb
