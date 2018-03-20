##############################################################################
# Copyright (c) 2018 sysmocom - s.f.m.c. GmbH
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# which accompanies this distribution, and is available at
# http://www.eclipse.org/legal/epl-v10.html
#
# Contributors:
#   Neels Hofmeyr
#
##############################################################################
# Makefile for the SummaryLogger plugin.  Unfortunately, we'll need four different
# libraries for each plugin...  Plugin version information must be
# synchronized with the code.

TOP    := .
include   Makefile.cfg

LIB_DIR := /usr/lib/titan
INCLUDE_DIR := /usr/include/titan

MAJOR := 0
MINOR := 0

SOURCES := SummaryLogger.cc
STATIC_SOURCES := ${SOURCES}
HEADERS := $(SOURCES:.cc=.hh)
OBJECTS := $(SOURCES:.cc=.o)
OBJECTS_RT2 := $(addprefix FT/,$(OBJECTS))

SHARED_LIB              := libsummarylogger.so
SHARED_LIB_RT2          := libsummarylogger-rt2.so
SHARED_LIB_PARALLEL     := libsummarylogger-parallel.so
SHARED_LIB_PARALLEL_RT2 := libsummarylogger-parallel-rt2.so

CPPFLAGS += -I$(INCLUDE_DIR)

CXXFLAGS += -I$(INCLUDE_DIR) -DLINUX

LDFLAGS_ORIG	     := $(LDFLAGS)
LDFLAGS              += -g -L/usr/lib/titan  -Wl,-soname,$(SHARED_LIB).$(MAJOR)				    -o $(SHARED_LIB).$(MAJOR).$(MINOR)
LDFLAGS_RT2          += $(LDFLAGS_ORIG) -g -L$(LIB_DIR) -Wl,-soname,$(SHARED_LIB_RT2).$(MAJOR)          -o $(SHARED_LIB_RT2).$(MAJOR).$(MINOR)
LDFLAGS_PARALLEL     += $(LDFLAGS_ORIG) -g -L$(LIB_DIR) -Wl,-soname,$(SHARED_LIB_PARALLEL).$(MAJOR)     -o $(SHARED_LIB_PARALLEL).$(MAJOR).$(MINOR)
LDFLAGS_PARALLEL_RT2 += $(LDFLAGS_ORIG) -g -L$(LIB_DIR) -Wl,-soname,$(SHARED_LIB_PARALLEL_RT2).$(MAJOR) -o $(SHARED_LIB_PARALLEL_RT2).$(MAJOR).$(MINOR)

LIBS              := -lttcn3-dynamic
LIBS_RT2          := -lttcn3-rt2-dynamic
LIBS_PARALLEL     := -lttcn3-parallel-dynamic
LIBS_PARALLEL_RT2 := -lttcn3-rt2-parallel-dynamic

TARGETS := $(SHARED_LIB) $(SHARED_LIB_PARALLEL) $(SHARED_LIB_RT2) $(SHARED_LIB_PARALLEL_RT2)
# .so with .major appended:
TARGETS_MAJOR       := $(addsuffix .$(MAJOR), $(TARGETS))
# .so with .major.minor appended:
TARGETS_MAJOR_MINOR := $(addsuffix .$(MINOR), $(TARGETS_MAJOR))

# OBJECTS_RT2, TARGETS_MAJOR and TARGETS_MAJOR_MINOR are non-standard make variables,
# not taken into account by "clean" in Makefile.genrules
# Delete them as "miscellaneous" files.
TOBECLEANED := $(OBJECTS_RT2) $(TARGETS_MAJOR) $(TARGETS_MAJOR_MINOR)

all run: $(TARGETS)

$(SHARED_LIB): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $? $(LIBS) -shared
	ln -sf $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	ln -sf $@.$(MAJOR) $@

$(SHARED_LIB_RT2): $(OBJECTS_RT2)
	$(CXX) $(CPPFLAGS_RT2) $(CXXFLAGS) $(LDFLAGS_RT2) $? $(LIBS_RT2) -shared
	ln -sf $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	ln -sf $@.$(MAJOR) $@

$(SHARED_LIB_PARALLEL): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS_PARALLEL) $? $(LIBS_PARALLEL) -shared
	ln -sf $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	ln -sf $@.$(MAJOR) $@

$(SHARED_LIB_PARALLEL_RT2): $(OBJECTS_RT2)
	$(CXX) $(CPPFLAGS_RT2) $(CXXFLAGS) $(LDFLAGS_PARALLEL_RT2) $? $(LIBS_PARALLEL_RT2) -shared
	ln -sf $@.$(MAJOR).$(MINOR) $@.$(MAJOR)
	ln -sf $@.$(MAJOR) $@

$(OBJECTS): $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $?

# The `-o $@' stuff is necessary, otherwise the result will be put into the
# current directory.
$(OBJECTS_RT2): $(SOURCES)
	mkdir -p FT
	$(CXX) $(CPPFLAGS_RT2) $(CXXFLAGS) -c $? -o $@

dep:
	@echo Doing nothing...

install: $(SHARED_LIB) $(SHARED_LIB_RT2) $(SHARED_LIB_PARALLEL) $(SHARED_LIB_PARALLEL_RT2)
	mkdir -p $(LIB_DIR)
	cp $(SHARED_LIB)* $(SHARED_LIB_RT2)* $(SHARED_LIB_PARALLEL)* $(SHARED_LIB_PARALLEL_RT2)* $(LIB_DIR)

include Makefile.genrules
