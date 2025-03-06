# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run
RUST_LOG := error

all: build
	@:

build:
	@printf "MK\tjson2md\n"
	@cargo build -q --release

clean:
	@cargo clean

run: build
	@

.PHONY: $(PHONY)
