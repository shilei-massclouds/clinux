# SPDX-License-Identifier: GPL-2.0
ARCH ?= riscv64

export MAKE := @make --no-print-directory
export KMODULE_DIR = ./target/_bootrd

# QEMU
QEMU := qemu-system-$(ARCH)
QEMU_ARGS := -m 512M
QEMU_ARGS += \
	-machine virt \
	-bios default \
	-kernel $(KMODULE_DIR)/startup.bin \
    -drive if=pflash,file=$(KMODULE_DIR)/bootrd.disk,format=raw,unit=1 \
	-device virtio-blk-device,drive=disk0 \
	-drive id=disk0,if=none,format=raw,file=/tmp/disk.img \
	-device virtio-net-device,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::5555-:5555 \
	-nographic \
    -append "earlycon=sbi root=/dev/vda rw console=ttyS0" \
    #-d in_asm -D /tmp/insturction.log

# All component subdir
components := \
	prebuilt startup
#	prebuilt startup lib \
#	rbtree radix_tree bitmap xarray hashtable \
#	early_dt of of_irq platform \
#	mm memblock buddy slab \
#	kalloc pgalloc gup readahead \
#	sched fork workqueue \
#	kobject irq softirq devres \
#	of_serial \
#	block genhd backing_dev \
#	bio iov_iter scatterlist mempool \
#	vma filemap ioremap \
#	intc plic \
#	virtio virtio_mmio virtio_blk \
#	dcache \
#	fs rootfs ramfs ext2 procfs \
#	sys userboot \
#	top_helloworld top_linux

all: build

build: tools bootrd
	@ ./tools/mk_bootrd/mk_bootrd

tools:
	$(MAKE) -C ./tools

bootrd: $(components)

$(components): predirs FORCE
	@mkdir -p ./target/$@
	$(MAKE) -f ./scripts/Makefile.build obj=$@

predirs:
	@mkdir -p ./target/_bootrd

run: build
	$(QEMU) $(QEMU_ARGS)

clean:
	@rm -rf ./target
	$(MAKE) -C ./tools clean

FORCE:

.PHONY: all build components predirs tools clean FORCE
