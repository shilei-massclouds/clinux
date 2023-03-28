# SPDX-License-Identifier: GPL-2.0

# Default module output directory
export KMODULE_DIR = ../output

# All component subdir
components := startup lib
predirs := prebuilt

build: $(components)
	@:

$(components): $(predirs) FORCE
	@mkdir -p ./target/$@
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

$(predirs): FORCE
	@mkdir -p ./target/$@
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

clean:
	@rm -f `find $(KMODULE_DIR) ! -name README.md -type f`
	@rm -rf ./target

FORCE:

.PHONY: build components predirs FORCE
