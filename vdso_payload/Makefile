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

all: $(TARGET_DIR)/vdso.so $(TARGET_DIR)/vdso-syms.o

$(TARGET_DIR)/vdso-syms.o: $(TARGET_DIR)/vdso-dummy.o FORCE
	@$(LD) -r --just-symbols $< -o $@
	@mkdir -p ./target/vdso
	@cp $@ ./target/vdso/

$(TARGET_DIR)/vdso-dummy.o: $(TARGET_DIR)/vdso.lds $(TARGET_DIR)/rt_sigreturn.o FORCE
	@$(CC) $(CFLAGS) -no-pie -nostdlib -nostartfiles -Wl,-T,$(filter-out FORCE,$^) -o $@.tmp && \
		$(OBJCOPY) $(patsubst %, -G __vdso_%, $(vdso-syms)) $@.tmp $@ && \
		rm $@.tmp

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
