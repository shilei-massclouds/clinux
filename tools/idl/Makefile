# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

TARGET := target/debug/idl

all: $(TARGET)
	@:

$(TARGET): src/main.rs src/grammar/*.pest src/grammar/*.tpl
	@printf "MK\t$@\n"
	@cargo build

clean:
	@:

run:
	@:

.PHONY: $(PHONY)
