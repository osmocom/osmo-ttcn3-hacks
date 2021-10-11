#!/bin/sh

# Copyright 2017-2019 Harald Welte
# Copyright 2018 sysmocom - s.f.m.c. GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Wrapper around the TITAN make file generator to work in Debian.
#
# TITAN has a makefile generator, but somehow Debian seems to install
# the binaries to different paths without patching the make file
# generator, leading in inconsistent non-working Makefiles.
#
# See https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=884303 for details.
#

test -x "$(which ttcn3_makefilegen 2>/dev/null)" || { echo "ERROR: ttcn3_makefilegen not in PATH"; exit 1; }

# Enable ccache if it can be found in path.
# This speeds up repeated builds of the TTCN3 tests by an order of magnitude
# since most of the generated C++ source files don't change very often.
# Roughly, for an initial build which takes N minutes, a complete rebuild
# after 'make clean' will only take N seconds with ccache.
# Note that ccache cannot speed up compilation of .o files to .so files.
if [ -z "$USE_CCACHE" ] && which ccache 2>/dev/null; then
	USE_CCACHE=1
fi

ttcn3_makefilegen -g -p -l -U 5 -f $*

sed -i -e 's/# TTCN3_DIR = /TTCN3_DIR = \/usr/' Makefile
sed -i -e 's/LDFLAGS = /LDFLAGS = -L \/usr\/lib\/titan/' Makefile
sed -i -e 's/LINUX_LIBS = -lxml2/LINUX_LIBS = -lxml2 -lsctp/' Makefile
#sed -i -e 's/TTCN3_LIB = ttcn3-parallel/TTCN3_LIB = ttcn3/' Makefile

# The -DMAKEDEPEND_RUN is a workaround for Debian packaging issue,
# see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=879816 for details
sed -i -e 's/CPPFLAGS = -D$(PLATFORM)/CPPFLAGS = -D$(PLATFORM) -DMAKEDEPEND_RUN -DUSE_SCTP -DLKSCTP_MULTIHOMING_ENABLED/' Makefile

#remove -Wall from CXXFLAGS: we're not interested in generic warnings for autogenerated code cluttering the logs
sed -i -e 's/-Wall//' Makefile

if [ "x$CPPFLAGS_TTCN3" != "x" ]; then
	sed -i -e 's/CPPFLAGS_TTCN3 =/CPPFLAGS_TTCN3 = '"$CPPFLAGS_TTCN3"'/' Makefile
fi

# for TITAN 6.3.0
if cat /etc/issue | grep "Arch Linux" >/dev/null 2>&1; then
        sed -i -e 's/TTCN3_DIR = $/TTCN3_DIR = \/usr\/ttcn3/' Makefile
else
        sed -i -e 's/TTCN3_DIR = $/TTCN3_DIR = \/usr/' Makefile
fi
sed -i -e 's/\/bin\/compiler/\/bin\/ttcn3_compiler/' Makefile

if [ "x$USE_CCACHE" = "x1" ]; then
	# enable ccache
	sed -i -e 's/^CXX = g++ $/CXX = env CCACHE_SLOPPINESS=time_macros ccache g++/' Makefile
	# Append the -D option to compiler flags. This option disables timestamps
	# inside comments in the generated C++ code which interfere with ccache.
	sed -i -e 's/^COMPILER_FLAGS = \(.*\)/& -D/' Makefile
fi
