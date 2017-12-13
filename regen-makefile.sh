#!/bin/sh

# Wrapper around the TITAN make file generator to work in Debian.
#
# TITAN has a makefile generator, but somehow Debian seems to install
# the binaries to different paths without patching the make file
# generator, leading in inconsistent non-working Makefiles.
#
# The regexes below patch the generated Makefile to work on Debian 9 and
# unstable, so far tested with TITAN 6.1.0, 6.2.0 and 6.3.0
#

ttcn3_makefilegen -l -f $*
sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan /' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile

# The -DMAKEDEPEND_RUN is a workaround for Debian packaging issue,
# see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=879816 for details
sed -i -e 's/CPPFLAGS = -D$(PLATFORM) -I$(TTCN3_DIR)\/include/CPPFLAGS = -D$(PLATFORM) -DMAKEDEPEND_RUN -I$(TTCN3_DIR)\/include -I\/usr\/include\/titan/' Makefile

# for TITAN 6.3.0
sed -i -e 's/TTCN3_DIR = $/TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/\/bin\/compiler/\/bin\/ttcn3_compiler/' Makefile
