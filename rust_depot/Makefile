# Arguments
ARCH ?= riscv64
MODE ?= release
LOG ?= warn

FS ?= off
NET ?= off
GRAPHIC ?= off

# Platform
PLATFORM ?= qemu-virt-riscv
target := riscv64gc-unknown-none-elf

# Default module output directory
export KMODULE_DIR = ../output

export ARCH
export PLATFORM
export MODE
export LOG

export NM = riscv64-linux-gnu-nm
export CC = riscv64-linux-gnu-gcc
export CFLAGS = -nostdinc -fno-PIE -mabi=lp64d -march=rv64imafdc \
	-fno-common -fno-stack-protector -mcmodel=medany -D__KERNEL__

# Allows the usage of unstable features in stable compilers.
export RUSTC_BOOTSTRAP := 1

# Paths
kernel_elf := target/$(target)/$(MODE)/startup
kernel_bin := $(kernel_elf).bin
kernel_map := $(kernel_elf).map

# Cargo features and build args

features := #axhal/platform-qemu-virt-riscv
features += #axdriver?/bus-mmio

ifneq ($(filter $(LOG),off error warn info debug trace),)
  features += #libax/log-level-$(LOG)
else
  $(error "LOG" must be one of "off", "error", "warn", "info", "debug", "trace")
endif

ifeq ($(FS), on)
  features += libax/fs
endif
ifeq ($(NET), on)
  features += libax/net
endif
ifeq ($(GRAPHIC), on)
  features += libax/display
endif

build_args := --features "$(features)" --target $(target) -Zbuild-std=core,alloc -Zbuild-std-features=compiler-builtins-mem
ifeq ($(MODE), release)
  build_args += --release
endif

# Binutils
OBJDUMP := rust-objdump -d --print-imm-hex --x86-asm-syntax=intel
OBJCOPY := rust-objcopy --binary-architecture=$(ARCH)
GDB := gdb-multiarch

# QEMU
qemu := qemu-system-$(ARCH)
qemu_args := -m 128M
qemu_args += \
	-machine virt \
	-bios default \
	-kernel $(kernel_bin) \
    -drive if=pflash,file=../output/bootrd.disk,format=raw,unit=1 \
	-device virtio-blk-device,drive=disk0 \
	-drive id=disk0,if=none,format=raw,file=/tmp/disk.img \
	-device virtio-net-device,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::5555-:5555 \
	-nographic \
    -d in_asm -D /tmp/insturction.log

build: $(kernel_bin)
	@cp $(kernel_map) ../output/
	@cp $(kernel_bin) ../output/

build_elf:
	@echo Arch: $(ARCH), Platform: $(PLATFORM)
	cargo build $(build_args)

$(kernel_bin): build_elf $(kernel_map)
	@$(OBJCOPY) $(kernel_elf) --strip-all -O binary $@

$(kernel_map): $(kernel_elf)
	@$(NM) -n $< | \
		grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)\|\( \.L\)' > $@

disasm:
	$(OBJDUMP) $(kernel_elf) | less

run: build justrun

justrun:
	$(qemu) $(qemu_args)

debug: build
	$(qemu) $(qemu_args) -s -S &
	sleep 1
	$(GDB) $(kernel_elf) -ex 'target remote localhost:1234'

clean:
	@cargo clean

clippy:
	cargo clippy --target $(target)

fmt:
	cargo fmt --all

test:
	cargo test --workspace --exclude "arceos-*" --exclude "libax_bindings" -- --nocapture

test_no_fail_fast:
	cargo test --workspace --exclude "arceos-*" --exclude "libax_bindings" --no-fail-fast -- --nocapture

$(KMODULE_DIR)/bootrd.disk:
	@ ../tools/mk_bootrd/mk_bootrd

bootrd: $(KMODULE_DIR)/bootrd.disk
	@:

.PHONY: build build_elf kernel_map disasm run justrun clean clippy fmt test test_no_fail_fast bootrd
