# SPDX-License-Identifier: GPL-2.0

include scripts/Makefile.include

PHONY := all clean

PREDIRS := prebuilt

SUBDIRS := startup

CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS))

PHONY += $(SUBDIRS)
PHONY += $(PREDIRS)

all: $(SUBDIRS) startup/startup.bin
	@cp ./startup/System.map ./output/
	@cp ./startup/startup.bin ./output/

$(PREDIRS):
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

$(SUBDIRS): $(PREDIRS)
	@$(MAKE) -f ./scripts/Makefile.build obj=$@
	$(if $(filter-out startup, $@), @cp ./$@/*.ko ./output/)

PHONY += $(CLEAN_DIRS)

$(CLEAN_DIRS):
	@$(MAKE) -f ./scripts/Makefile.clean obj=$@

clean: $(CLEAN_DIRS)
	@rm -f ./prebuilt/*.h ./prebuilt/*.s

dump:
	$(OBJDUMP) -D -m riscv:rv64 -EL -b binary ./startup/startup.bin

.PHONY: $(PHONY)
