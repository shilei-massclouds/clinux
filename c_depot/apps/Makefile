# SPDX-License-Identifier: GPL-2.0

export top = ..
export KMODULE_DIR = $(top)/../output/apps

apps := \
	sqlite

build: $(apps)
	@:

$(apps): FORCE
	@mkdir -p ./target/$@
	@$(MAKE) -f ../scripts/Makefile.build obj=$@

clean:
	@rm -f `find $(KMODULE_DIR) ! -name README.md -type f`
	@rm -rf ./target

FORCE:

.PHONY: build apps FORCE
