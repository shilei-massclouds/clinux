# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run

all: build
	@:

build:
	@printf "MK\tfind_dep\n"
	@cargo build --release

clean:
	@cargo clean

run: build
	@./target/release/find_dep ../../target/_bootrd/ top_early_printk

.PHONY: $(PHONY)
