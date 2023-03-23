# SPDX-License-Identifier: GPL-2.0

obj_y = init.o

CROSS_ := riscv64-linux-gnu-

CC  := $(CROSS_)gcc
AR  := $(CROSS_)ar
LD  := $(CROSS_)ld
NM  := $(CROSS_)nm

INCLUDES := -I../include/
CFLAGS := -nostdinc -fno-PIE -mabi=lp64d -march=rv64imafdc \
	-fno-common -fno-stack-protector -mcmodel=medany -D__KERNEL__

TARGET_DIR = target/$(target)/$(MODE)/deps
rust_rlib = $(TARGET_DIR)/lib$(obj)-*.rlib
rust_obj_y = $(TARGET_DIR)/$(obj)-*.o
obj_y := $(addprefix $(TARGET_DIR)/, $(obj_y))
mod_lds := scripts/module.lds

_mod: $(TARGET_DIR)/$(obj).ko
	@printf "CP\t$<\n"
	@cp $< $(KMODULE_DIR)

$(TARGET_DIR)/$(obj).ko: $(rust_obj_y) $(obj_y) $(mod_lds)
	@printf "LD\t$@\n"
	$(LD) -r $(LDFLAGS) -T $(mod_lds) -o $@ $(rust_obj_y) $(obj_y)

$(rust_obj_y): $(rust_rlib)
	@printf "AR\t$@\n"
	$(AR) -x $(rust_rlib) --output $(TARGET_DIR)/

$(TARGET_DIR)/%.o: $(obj)/src/%.c
	@printf "CC\t$<\n"
	$(CC) $(CFLAGS) $(CFLAGS_$(@F)) $(INCLUDES) -c -o $@ $<

.PHONY: _mod rust_obj_y

#############################

always_y := exports_axruntime_generated.h

