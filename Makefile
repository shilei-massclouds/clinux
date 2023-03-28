# SPDX-License-Identifier: GPL-2.0

.PHONY: all tools clean dump run bootrd FORCE

MAKE := @make --no-print-directory

all: tools
	@$(MAKE) -C c_depot

tools: FORCE
	@$(MAKE) -C tools

run: tools
	@$(MAKE) -C c_depot run

bootrd: tools
	@$(MAKE) -C c_depot bootrd

clean:
	@$(MAKE) -C tools clean
	@$(MAKE) -C c_depot clean

dump:
	$(OBJDUMP) -D -m riscv:rv64 -EL -b binary ./startup/startup.bin
