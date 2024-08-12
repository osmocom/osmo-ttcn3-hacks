ARG	REGISTRY=docker.io
ARG	DISTRO=debian:12
FROM	${REGISTRY}/${DISTRO}

COPY	obs.key /obs.key
COPY	setup.sh /setup.sh
RUN	sh -ex /setup.sh

# Make apt noninteractive (for commands running after the container was built)
ENV	DEBIAN_FRONTEND=noninteractive

# Ccache is installed above so it can be optionally used when rebuilding the
# testsuites inside the docker containers. Don't use it by default.
ENV	USE_CCACHE=0
