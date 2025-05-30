# Copyright 2017 Harald Welte
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

BUILDDIR ?= _build

SUBDIRS= \
	5gc \
	asterisk \
	bsc \
	bsc-nat \
	bts \
	cbc \
	ccid \
	dia2gsup \
	eim \
	fr \
	fr-net \
	epdg \
	gbproxy \
	ggsn_tests \
	hlr \
	hnbgw \
	hnodeb \
	hss \
	ipad \
	mgw \
	mme \
	msc \
	ns \
	pcap-client \
	pcu \
	pcrf \
	pgw \
	remsim \
	s1gw \
	sccp \
	selftest \
	sgsn \
	simtrace \
	sip \
	smlc \
	stp \
	sysinfo \
	upf \
	$(NULL)

NPROC=$(shell nproc 2>/dev/null)
ifeq ($(NPROC),)
NPROC=1
endif
PARALLEL_MAKE ?= -j$(NPROC)

# This master makefile allows you to do things like
# 	make clean	(remove all generated binary, c++ and symlinks)
# 	make compile	(compile ttcn3 into c++)
# 	make all	(compile c++ into executable)
#
# as well as per-subdirectory targets like
#
#	make bsc/clean
#	make bsc/compile
#	make bsc/all
#	make bsc	(equivalent to bsc/all)

default: deps all

# Eclipse GitLab has rate limiting and sometimes to many concurrent conns fail.
# If -jN fails, retry with -j1.
.make.deps: deps/Makefile
	($(MAKE) $(PARALLEL_MAKE) -C deps || $(MAKE) -j1 -C deps)
	touch $@

.PHONY: deps
deps: .make.deps

# deps-update target for backwards compat; now does the same as 'make deps'
.PHONY: deps-update
deps-update: .make.deps

compile: $(foreach dir,$(SUBDIRS),$(dir)/compile)
clean-old: $(foreach dir,$(SUBDIRS),$(dir)/clean-old)
	@rm -rfv ~/.cache/osmo-ttcn3-testenv/podman/osmo-ttcn3-hacks
all: $(foreach dir,$(SUBDIRS),$(dir)/all)

define DIR_Makefile_template
$(BUILDDIR)/$(1)/Makefile: $(1)/gen_links.sh $(1)/regen_makefile.sh
	(cd $(1) && ./gen_links.sh && ./regen_makefile.sh)
endef

define DIR_compile_template
.PHONY: $(1)/compile
$(1)/compile: deps $(BUILDDIR)/$(1)/Makefile
	$(MAKE) -C $(BUILDDIR)/$(1) compile
endef

# Remove left-over files from before the buildsystem was changed to do
# out-of-tree builds. _buildsystem/gen_links.inc.sh tells the users to run this
# if needed.
define DIR_clean_old_template
.PHONY: $(1)/clean-old
$(1)/clean-old:
	@git clean -fx "$(1)"
endef

define DIR_clean_template
.PHONY: $(1)/clean
$(1)/clean:
	$(MAKE) -C $(BUILDDIR)/$(1) clean
endef

define DIR_all_template
$(1): $(1)/all
.PHONY: $(1)/all
$(1)/all: deps $(BUILDDIR)/$(1)/Makefile
	$(MAKE) -C $(BUILDDIR)/$(1) compile
	$(MAKE) $(PARALLEL_MAKE) -C $(BUILDDIR)/$(1)
endef

$(foreach dir,$(SUBDIRS), \
	$(eval $(call DIR_Makefile_template,$(dir)))	\
	$(eval $(call DIR_compile_template,$(dir)))	\
	$(eval $(call DIR_clean_old_template,$(dir)))	\
	$(eval $(call DIR_clean_template,$(dir)))	\
	$(eval $(call DIR_all_template,$(dir)))		\
	)

.PHONY: tags regen-diameter-types-ttcn clean
tags:
	find $(shell pwd) \
		-type f -name "*.ttcn" -o \
		-type f -name "*.ttcnpp" | \
	xargs ctags

regen-diameter-types-ttcn:
	(cd library/ && ./regen-DIAMETER_Types_ttcn.sh)

# Intentionally not using $(BUILDDIR) here to avoid user errors leading to
# unintentional removal of other files. If $(BUILDDIR) is changed, it is
# trivial to clean up the builddir manually.
clean:
	rm -rf _build
