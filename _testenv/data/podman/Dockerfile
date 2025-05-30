ARG	REGISTRY=docker.io
ARG	DISTRO=debian:12
FROM	${REGISTRY}/${DISTRO}

# Arguments used after FROM must be specified again
ARG	OSMOCOM_REPO_TESTSUITE_MIRROR="https://downloads.osmocom.org"
ARG	OSMOCOM_REPO="$OSMOCOM_REPO_TESTSUITE_MIRROR/packages/osmocom:/latest/Debian_12/"

# Copy from common dir
COPY	obs.key /obs.key

# Install packages from Debian repositories (alphabetic order)
ENV	DEBIAN_FRONTEND=noninteractive
RUN	set -x && \
	mkdir -p /etc/kernel/postinst.d && \
	touch /etc/kernel/postinst.d/initramfs-tools && \
	apt-get update && \
	apt-get install \
		-y \
		--no-install-recommends \
		-o Dpkg::Options::="--force-confold" \
		apt-rdepends \
		autoconf \
		automake \
		bc \
		bison \
		bridge-utils \
		build-essential \
		busybox-static \
		ca-certificates \
		ccache \
		cmake \
		cpio \
		erlang-dev \
		erlang-nox \
		flex \
		gdb \
		git \
		gzip \
		iproute2 \
		iputils-ping \
		libbson-dev \
		libc-ares-dev \
		libcsv-dev \
		libcurl4-gnutls-dev \
		libdbd-sqlite3 \
		libdbi-dev \
		libelf-dev \
		libgcc-11-dev \
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
		linux-image-amd64 \
		meson \
		netcat-openbsd \
		pax-utils \
		pcscd \
		pkg-config \
		procps \
		psmisc \
		python3-pip \
		qemu-system-x86 \
		rsync \
		source-highlight \
		sqlite3 \
		sudo \
		tcpdump \
		tree \
		vim \
		vsmartcard-vpcd \
		wget \
		wireshark-common \
		&& \
	apt-get clean

# Install newer libcurl4 from bookworm-backports, libcurl4 7.88.1-10+deb12u12
# shipped with debian12 is buggy and generates wrong HTTP2 for open5gs.
ENV	BACKPORTS_SOURCES_LIST="/etc/apt/sources.list.d/debian-backports.sources"
RUN	set -x && \
	echo "Types: deb deb-src" >>"$BACKPORTS_SOURCES_LIST" && \
	echo "URIs: http://deb.debian.org/debian" >>"$BACKPORTS_SOURCES_LIST" && \
	echo "Suites: stable-backports" >>"$BACKPORTS_SOURCES_LIST" && \
	echo "Components: main" >>"$BACKPORTS_SOURCES_LIST" && \
	echo "Enabled: yes" >>"$BACKPORTS_SOURCES_LIST" && \
	echo "Signed-By: /usr/share/keyrings/debian-archive-keyring.gpg" >>"$BACKPORTS_SOURCES_LIST" && \
	apt-get update && \
	apt-get -t bookworm-backports install \
		-y \
		--no-install-recommends \
		-o Dpkg::Options::="--force-confold" \
		libcurl4 \
		&& \
	apt-get clean

# Install rebar3 as described in https://rebar3.org/docs/getting-started/
# instead of using the Debian package, as the latter pulls in ~600 MB of GUI
# dependencies that we don't need:
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1083096
RUN	wget https://github.com/erlang/rebar3/releases/download/3.24.0/rebar3 -O /usr/bin/rebar3 && \
	chmod +x /usr/bin/rebar3 && \
	rebar3 --version

# Ccache is installed above so it can be optionally used when rebuilding the
# testsuites inside the docker containers. Don't use it by default.
ENV	USE_CCACHE=0

# Binary-only transcoding library for RANAP/RUA/HNBAP to work around TITAN only implementing BER
RUN	set -x && \
	export DPKG_ARCH="$(dpkg --print-architecture)" && \
	wget https://ftp.osmocom.org/binaries/libfftranscode/libfftranscode0_0.5_${DPKG_ARCH}.deb && \
	wget https://ftp.osmocom.org/binaries/libfftranscode/libfftranscode-dev_0.5_${DPKG_ARCH}.deb && \
	dpkg -i ./libfftranscode0_0.5_${DPKG_ARCH}.deb ./libfftranscode-dev_0.5_${DPKG_ARCH}.deb && \
	apt-get install --fix-broken && \
	rm libfftranscode*.deb

# Install osmo-python-tests (for obtaining talloc reports from SUT)
ADD	https://gerrit.osmocom.org/plugins/gitiles/python/osmo-python-tests/+/master?format=TEXT /tmp/osmo-python-tests-commit
RUN	set -x && \
	git clone --depth=1 https://gerrit.osmocom.org/python/osmo-python-tests osmo-python-tests && \
	pip3 install ./osmo-python-tests --break-system-packages && \
	rm -rf osmo-python-tests

# Add eclipse-titan from osmocom:latest, invalidate cache when :latest changes
RUN	echo "deb [signed-by=/obs.key] $OSMOCOM_REPO ./" \
		> /etc/apt/sources.list.d/osmocom-latest.list
ADD	$OSMOCOM_REPO/Release /tmp/Release
RUN	set -x && \
	apt-get update && \
	apt-get install -y --no-install-recommends \
		eclipse-titan \
		&& \
	apt-get clean && \
	rm /etc/apt/sources.list.d/osmocom-latest.list

# Add mongodb for open5gs-hss. Using the package from bullseye since bookworm
# mongodb-org package is not available. Furthermore, manually install required
# libssl1.1.
RUN	set -x && \
	mkdir -p /tmp/mongodb && \
	cd /tmp/mongodb && \
	wget "https://pgp.mongodb.com/server-5.0.asc" -O "/mongodb.key" && \
	wget "http://security.debian.org/debian-security/pool/updates/main/o/openssl/libssl1.1_1.1.1n-0+deb10u6_amd64.deb" && \
	dpkg -i "libssl1.1_1.1.1n-0+deb10u6_amd64.deb" && \
	echo "deb [signed-by=/mongodb.key] http://repo.mongodb.org/apt/debian bullseye/mongodb-org/5.0 main" \
		> /etc/apt/sources.list.d/mongodb-org.list && \
	apt-get update && \
	apt-get install -y mongodb-org && \
	apt-get clean && \
	cd / && \
	rm -rf /tmp/mongodb && \
	rm /etc/apt/sources.list.d/mongodb-org.list
