# SPDX-License-Identifier: GPL-2.0
ARCH ?= riscv64

export MAKE := @make --no-print-directory

# Default module output directory
export KMODULE_DIR = ../output

# QEMU
qemu := qemu-system-$(ARCH)
qemu_args := -m 512M
qemu_args += \
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

predirs := prebuilt

# All component subdir
components := \
	startup lib \
	rbtree radix_tree bitmap xarray hashtable \
	early_dt of of_irq platform \
	mm memblock buddy slab \
	kalloc pgalloc gup readahead \
	sched fork workqueue \
	kobject irq softirq devres \
	of_serial \
	block genhd backing_dev \
	bio iov_iter scatterlist mempool \
	vma filemap ioremap \
	intc plic \
	virtio virtio_mmio virtio_blk \
	dcache \
	fs rootfs ramfs ext2 procfs \
	sys userboot \
	top_helloworld top_linux

build: $(components)
	@:

$(components): $(predirs) FORCE
	@mkdir -p ./target/$@
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

$(predirs): FORCE
	@mkdir -p ./target/$@
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

run: build bootrd justrun

justrun:
	@ $(qemu) $(qemu_args)

clean:
	@rm -f `find $(KMODULE_DIR) ! -name README.md -type f`
	@rm -rf ./target

bootrd: FORCE
	@rm -f $(KMODULE_DIR)/bootrd.disk $(KMODULE_DIR)/top_*.json
	@ ../tools/mk_bootrd/mk_bootrd

profiles:
	@ ../tools/ch_bootrd/ch_bootrd $(KMODULE_DIR)/bootrd.disk

FORCE:

.PHONY: build components predirs clean FORCE
