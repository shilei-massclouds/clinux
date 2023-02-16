# SPDX-License-Identifier: GPL-2.0

include scripts/Makefile.include

PHONY := all clean dump run profiles bootrd

# Default module output directory
export KMODULE_DIR := ./output

# Allows the usage of unstable features in stable compilers.
export RUSTC_BOOTSTRAP := 1

PREDIRS := prebuilt

SUBDIRS := \
	startup lib rbtree radix_tree hashtable bitmap \
	workqueue scatterlist xarray mempool \
	early_dt of of_serial of_irq platform \
	intc plic irq softirq \
	mm memblock pgalloc buddy slab kalloc \
	vma ioremap gup devres \
	fs dcache filemap readahead \
	kobject bio iov_iter block genhd backing-dev \
	fork sched \
	virtio virtio_mmio virtio_blk \
	ext2 ramfs rootfs procfs \
	sys \
	userboot \
	top_linux top_hello_world rs_hello #rs_ext2

CLEAN_DIRS := $(addprefix _clean_, $(SUBDIRS) $(PREDIRS))

all: $(SUBDIRS) startup/startup.bin tools
	@cp ./startup/System.map $(KMODULE_DIR)
	@cp ./startup/startup.bin $(KMODULE_DIR)

PHONY += $(PREDIRS)
$(PREDIRS):
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

PHONY += $(SUBDIRS)
$(SUBDIRS): $(PREDIRS) rust
	@$(MAKE) -f ./scripts/Makefile.build obj=$@
	$(if $(filter-out startup, $@), @cp ./$@/*.ko $(KMODULE_DIR))

PHONY += rust
rust:
	@$(SHELL) ./scripts/rust_is_available.sh -v
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

PHONY += tools
tools:
	@$(MAKE) -C tools

PHONY += $(CLEAN_DIRS)
$(CLEAN_DIRS):
	@$(MAKE) -f ./scripts/Makefile.clean obj=$@

clean: $(CLEAN_DIRS)
	@rm -f ./prebuilt/*.h ./prebuilt/*.s
	@$(MAKE) -C tools clean
	@$(MAKE) -C rust clean
	@find $(KMODULE_DIR)/* | grep -v README.md | xargs rm -f

$(KMODULE_DIR)/bootrd.disk:
	@ ./tools/mk_bootrd/mk_bootrd

bootrd: $(KMODULE_DIR)/bootrd.disk
	@:

run: all bootrd
	@$(MAKE) -C tools run
	@ ./scripts/qemu.sh

profiles: bootrd
	@ ./tools/ch_bootrd/ch_bootrd $(KMODULE_DIR)/bootrd.disk

dump:
	$(OBJDUMP) -D -m riscv:rv64 -EL -b binary ./startup/startup.bin

.PHONY: $(PHONY)
