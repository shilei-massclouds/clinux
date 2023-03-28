# SPDX-License-Identifier: GPL-2.0

PHONY := all clean

SUBDIRS := mk_rootfs mk_bootrd ch_bootrd json2md idl

CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS))

all: $(SUBDIRS)
	@:

PHONY += $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@

PHONY += $(CLEAN_DIRS)
$(CLEAN_DIRS):
	@$(MAKE) -C $(subst _clean_, , $@) clean

clean: $(CLEAN_DIRS)

.PHONY: $(PHONY)
