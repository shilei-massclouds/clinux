# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

SUBDIRS := find_dep mk_rootfs json2md

CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS))
RUN_DIRS := $(addprefix _run_, $(SUBDIRS))

all: $(SUBDIRS)
	@:

PHONY += $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@

PHONY += $(CLEAN_DIRS)
$(CLEAN_DIRS):
	@$(MAKE) -C $(subst _clean_, , $@) clean

clean: $(CLEAN_DIRS)

PHONY += $(RUN_DIRS)
$(RUN_DIRS):
	@$(MAKE) -C $(subst _run_, , $@) run

run: $(RUN_DIRS)

.PHONY: $(PHONY)
