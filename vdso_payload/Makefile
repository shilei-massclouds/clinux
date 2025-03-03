# SPDX-License-Identifier: GPL-2.0

PHONY := all clean FORCE

include ./scripts/Makefile.include
include include/config/auto.conf

TARGET_DIR = ./target/$(obj)

# Symbols present in the vdso
vdso-syms  = rt_sigreturn
ifdef CONFIG_64BIT
vdso-syms += vgettimeofday
endif
vdso-syms += getcpu
vdso-syms += flush_icache

CFLAGS_vgettimeofday.o := -fPIC

# Files to link into the vdso
obj-vdso := $(patsubst %, %.o, $(vdso-syms)) note.o
obj-vdso := $(addprefix $(TARGET_DIR)/, $(obj-vdso))

all: $(TARGET_DIR)/vdso.so

$(TARGET_DIR)/vdso.so: $(TARGET_DIR)/vdso.so.dbg
	@printf "CP\t$<\n"
	@cp $< $@

$(TARGET_DIR)/vdso.so.dbg: $(TARGET_DIR)/vdso.lds $(obj-vdso) FORCE
	@$(CC) $(CFLAGS) -nostdlib -nostartfiles -Wl,-T,$(filter-out FORCE,$^) -o $@.tmp && \
		$(OBJCOPY) -S $(patsubst %, -G __vdso_%, $(vdso-syms)) $@.tmp $@ && \
		rm $@.tmp

$(TARGET_DIR)/vdso.lds: $(obj)/src/vdso.lds.S
	@printf "AS\t$<\n"
	@$(CPP) $(INCLUDES) -P -C -Uriscv -D__ASSEMBLY__ -o $@ $<

$(TARGET_DIR)/%.o: $(obj)/src/%.c
	@printf "CC\t$<\n"
	@$(CC) $(CFLAGS) $(CFLAGS_$(@F)) $(INCLUDES) -c -o $@ $<

$(TARGET_DIR)/%.o: $(obj)/src/%.S
	@printf "AS\t$<\n"
	@$(CC) $(AS_FLAGS) $(INCLUDES) -c -o $@ $<

FORCE:

.PHONY: $(PHONY)

#PHONY := all clean run
#
#SUBDIRS := find_dep mk_rootfs json2md
#
#CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS))
#RUN_DIRS := $(addprefix _run_, $(SUBDIRS))
#
#all: $(SUBDIRS)
#	@:
#
#PHONY += $(SUBDIRS)
#$(SUBDIRS):
#	@$(MAKE) -C $@
#
#PHONY += $(CLEAN_DIRS)
#$(CLEAN_DIRS):
#	@$(MAKE) -C $(subst _clean_, , $@) clean
#
#clean: $(CLEAN_DIRS)
#
#PHONY += $(RUN_DIRS)
#$(RUN_DIRS):
#	@$(MAKE) -C $(subst _run_, , $@) run
#
#run: $(RUN_DIRS)
#
#.PHONY: $(PHONY)
