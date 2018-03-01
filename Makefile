SUBDIRS=bsc bsc-nat bts ggsn_tests gprs_gb hlr lapdm mgw msc selftest sgsn sysinfo

PARALLEL_MAKE ?= -j8

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

.PHONY: deps
deps:
	$(MAKE) -C deps

.PHONY: deps-update
deps-update:
	$(MAKE) -C deps update

compile: $(foreach dir,$(SUBDIRS),$(dir)/compile)
clean: $(foreach dir,$(SUBDIRS),$(dir)/clean)
all: $(foreach dir,$(SUBDIRS),$(dir)/all)

define DIR_Makefile_template
$(1)/Makefile:
	(cd $(1) && ./gen_links.sh && ./regen_makefile.sh)
endef

define DIR_compile_template
.PHONY: $(1)/compile
$(1)/compile: deps $(1)/Makefile
	$(MAKE) -C $(1) compile
endef

define DIR_clean_template
.PHONY: $(1)/clean
$(1)/clean: $(1)/Makefile
	$(MAKE) -C $(1) clean
	(cd $(1) && ../rmlinks.sh && rm Makefile)
endef

define DIR_all_template
$(1): $(1)/all
.PHONY: $(1)/all
$(1)/all: deps $(1)/Makefile
	$(MAKE) -C $(1) compile
	$(MAKE) $(PARALLEL_MAKE) -C $(1)
endef

$(foreach dir,$(SUBDIRS), \
	$(eval $(call DIR_Makefile_template,$(dir)))	\
	$(eval $(call DIR_compile_template,$(dir)))	\
	$(eval $(call DIR_clean_template,$(dir)))	\
	$(eval $(call DIR_all_template,$(dir)))		\
	)
