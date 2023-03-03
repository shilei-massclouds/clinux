# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run TARGET

IDL := tools/idl/target/debug/idl

IDL_DIR := interfaces
RUST_ITF_DIR := rust/kernel/interfaces

files := ibase.rs ilib.rs

TARGET := $(addprefix $(obj)/, $(files))

all: $(TARGET)
	@:

$(obj)/%.rs: $(IDL_DIR)/%.idl
	@printf "IDL\t$@\n"
	$(IDL) -O $(RUST_ITF_DIR) -i $(basename $(notdir $@))

clean:
	@rm -f $(files)

run:
	@:

.PHONY: $(PHONY)
