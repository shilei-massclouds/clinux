# SPDX-License-Identifier: GPL-2.0
ARCH ?= riscv64

export MAKE := @make --no-print-directory
export KMODULE_DIR = ./target/_bootrd

TOP ?= top_early_printk
TOP_COMPONENT := $(TOP)

CROSS_ := riscv64-linux-gnu-
CPP := $(CC) -E
NM  := $(CROSS_)nm
LD  := $(CROSS_)ld
LDFLAGS := -melf64lriscv --build-id=none --strip-debug
OBJCOPY := $(CROSS_)objcopy
OBJCOPYFLAGS := -O binary -R .note -R .note.gnu.build-id -R .comment -S

INCLUDES := -isystem /usr/lib/gcc-cross/riscv64-linux-gnu/11/include -I./arch/riscv/include -I./arch/riscv/include/generated \
	-I./include -I./arch/riscv/include/uapi -I./arch/riscv/include/generated/uapi -I./include/uapi -I./include/generated/uapi \
	-include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h

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
    #-d in_asm -D /tmp/insturction.log

# All component subdir
components := \
	prebuilt booter \
	lib \
	early_printk top_early_printk \
	#top_lib \
	#printk top_printk \
	#top_booter

BLOCKS := booter.ko lib.ko early_printk.ko top_early_printk.ko
_BLOCKS := $(addprefix $(KMODULE_DIR)/, $(BLOCKS))

all: tools build

tools:

build: target/kernel.bin

target/kernel.bin: target/kernel.elf target/kernel.map
	@printf "Build kernel: $(TOP) ...\n"
	@$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

target/kernel.map: target/kernel.elf
	@$(NM) -n $< | \
		grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)\|\( \.L\)' > $@

target/kernel.elf: necessities target/booter.lds
	@printf "LD\t$@\n"
	@$(LD) $(LDFLAGS) -T target/booter.lds -o $@ $(_BLOCKS)

target/booter.lds: booter/src/booter.lds.S
	@printf "AS\t$<\n"
	@$(CPP) $(INCLUDES) -P -Uriscv -D__ASSEMBLY__ -o $@ $<

necessities: $(components)
	@printf "Discover necessities ...\n"

#build: tools bootrd
#	@ ./tools/mk_bootrd/mk_bootrd

tools:
	$(MAKE) -C ./tools

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

.PHONY: all build tools necessities components predirs clean FORCE
