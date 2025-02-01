# SPDX-License-Identifier: GPL-2.0
ARCH ?= riscv64

export MAKE := @make --no-print-directory
export KMODULE_DIR = $(CURDIR)/target/_bootrd/

TOP ?= top_early_printk
export TOP_COMPONENT := $(TOP)

include ./scripts/Makefile.include

# QEMU
QEMU := qemu-system-$(ARCH)
QEMU_ARGS := -m 512M
QEMU_ARGS += \
	-machine virt \
	-bios default \
	-kernel target/kernel.bin \
	-device virtio-blk-device,drive=disk0 \
	-drive id=disk0,if=none,format=raw,file=/tmp/disk.img \
	-device virtio-net-device,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::5555-:5555 \
	-nographic \
    -append "earlycon=sbi root=/dev/vda rw console=ttyS0" \
    -d in_asm -D /tmp/insturction.log

# All component subdir
components := \
	prebuilt booter lib early_printk

SELECTED = $(shell cat $(KMODULE_DIR)selected.in)
CL_INIT := $(KMODULE_DIR)cl_init

all: build

build: clean predirs tools target/kernel.bin

target/kernel.bin: target/kernel.elf target/kernel.map
	@printf "CP\t$@\n"
	@$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

target/kernel.map: target/kernel.elf
	@$(NM) -n $< | \
		grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)\|\( \.L\)' > $@

target/kernel.elf: target/vmlinux.lds $(CL_INIT).o
	@printf "LD\t$@\n"
	@$(LD) $(LDFLAGS) -T target/vmlinux.lds -o $@ $(SELECTED) $(CL_INIT).o

target/vmlinux.lds: booter/src/vmlinux.lds.S
	@printf "AS\t$<\n"
	@$(CPP) $(INCLUDES) -P -Uriscv -D__ASSEMBLY__ -o $@ $<

$(CL_INIT).o: $(CL_INIT).c
	@printf "CC\t$<\n"
	@$(CC) $(CFLAGS) $(CFLAGS_$(@F)) -DKBUILD_MODNAME='"$(obj)"' $(INCLUDES) -c -o $@ $<

$(CL_INIT).c: necessities

necessities: $(components) $(KMODULE_DIR)$(TOP_COMPONENT).ko
	@./tools/find_dep/target/release/find_dep $(KMODULE_DIR) $(TOP)

tools:
	$(MAKE) -C ./tools

$(components): FORCE
	@mkdir -p ./target/$@
	$(MAKE) -f ./scripts/Makefile.build obj=$@

$(KMODULE_DIR)$(TOP_COMPONENT).ko: FORCE
	@rm -rf ./target/top_component
	@mkdir -p ./target/top_component
	$(MAKE) -f ./scripts/Makefile.build obj=top_component
	@mv $(KMODULE_DIR)top_component.ko $(KMODULE_DIR)$(TOP_COMPONENT).ko

predirs:
	@mkdir -p ./target/_bootrd

run: build
	$(QEMU) $(QEMU_ARGS)

clean:
	@rm -rf ./target
	$(MAKE) -C ./tools clean

FORCE:

.PHONY: all build tools necessities components predirs clean FORCE
