# SPDX-License-Identifier: GPL-2.0

PHONY := all clean run
RUST_LOG := error

all: build
	@:

build:
	@printf "MK\tjson2dot (It may cost several seconds.)\n"
	@cargo build -q --release

clean:
	@cargo clean

run: build
	@

.PHONY: $(PHONY)
