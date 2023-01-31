# SPDX-License-Identifier: GPL-2.0

include scripts/Makefile.include

PHONY := all clean dump run

PREDIRS := prebuilt

SUBDIRS := \
	startup lib rbtree hashtable \
	workqueue scatterlist xarray mempool \
	early_dt \
	softirq \
	mm memblock pgalloc buddy slab kalloc gup \
	fs dcache filemap \
	kobject bio iov_iter block genhd backing-dev \
	readahead \
	fork sched \
	init

CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS) $(PREDIRS))

all: $(SUBDIRS) startup/startup.bin tools
	@cp ./startup/System.map ./output/
	@cp ./startup/startup.bin ./output/

PHONY += $(PREDIRS)
$(PREDIRS):
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

PHONY += $(SUBDIRS)
$(SUBDIRS): $(PREDIRS)
	@$(MAKE) -f ./scripts/Makefile.build obj=$@
	$(if $(filter-out startup, $@), @cp ./$@/*.ko ./output/)

PHONY += tools
tools:
	@$(MAKE) -C tools

PHONY += $(CLEAN_DIRS)
$(CLEAN_DIRS):
	@$(MAKE) -f ./scripts/Makefile.clean obj=$@

clean: $(CLEAN_DIRS)
	@rm -f ./prebuilt/*.h ./prebuilt/*.s
	@$(MAKE) -C tools clean
	@find ./output/* | grep -v README.md | xargs rm -f

run: all
	@$(MAKE) -C tools run
	@ ./scripts/qemu.sh

dump:
	$(OBJDUMP) -D -m riscv:rv64 -EL -b binary ./startup/startup.bin

.PHONY: $(PHONY)
