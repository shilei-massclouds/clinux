# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run
RUST_LOG := error

all: build
	@:

build:
	@printf "MK\tfind_dep\n"
	@cargo build -q --release

clean:
	@rm -f ./selected.in

run: build
	@./target/release/find_dep ../../target/_bootrd/ top_early_printk

.PHONY: $(PHONY)
