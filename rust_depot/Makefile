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
export target

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
export MAKE := @make --no-print-directory

# Binutils
export OBJDUMP = rust-objdump -d --print-imm-hex --x86-asm-syntax=intel
export OBJCOPY = rust-objcopy --binary-architecture=$(ARCH)
export GDB = gdb-multiarch

# Allows the usage of unstable features in stable compilers.
export RUSTC_BOOTSTRAP := 1

# Paths
kernel_elf := target/$(target)/$(MODE)/deps/startup/startup
kernel_bin := $(kernel_elf).bin
kernel_map := $(kernel_elf).map

# Cargo features and build args

features := axhal/platform-qemu-virt-riscv
#features += axdriver?/bus-mmio

ifneq ($(filter $(LOG),off error warn info debug trace),)
  #features += libax/log-level-$(LOG)
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

# QEMU
qemu := qemu-system-$(ARCH)
qemu_args := -m 128M
qemu_args += \
	-machine virt \
	-bios default \
	-kernel $(KMODULE_DIR)/startup.bin \
    -drive if=pflash,file=$(KMODULE_DIR)/bootrd.disk,format=raw,unit=1 \
	-device virtio-net-device,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::5555-:5555 \
	-nographic \
    #-d in_asm -D /tmp/insturction.log
	#-device virtio-blk-device,drive=disk0 \
	#-drive id=disk0,if=none,format=raw,file=/tmp/disk.img \

# All components subdir
components = \
	startup compiler_builtins core alloc \
	riscv log memory_addr page_table page_table_entry \
	spinlock smoltcp\
	bitmap_allocator buddy_system_allocator allocator \
	virtio_drivers driver_common driver_virtio \
	axhal axalloc axerror axlog axtask axruntime \
	axdriver axnet \
	libax \
	top_helloworld top_memtest top_exception \
	top_echoserver top_httpclient top_httpserver \
	top_parallel top_sleep top_yield

build: cargo_build $(components)
	@cp $(kernel_map) $(KMODULE_DIR)
	@cp $(kernel_bin) $(KMODULE_DIR)

cargo_build: FORCE
	@echo Arch: $(ARCH), Platform: $(PLATFORM)
	@cargo build $(build_args)

$(components): FORCE
	@mkdir -p ./target/$(target)/$(MODE)/deps/$@
	@$(MAKE) -f ./scripts/Makefile.build obj=$@

disasm:
	$(OBJDUMP) $(kernel_elf) | less

run: build bootrd justrun

justrun:
	@ $(qemu) $(qemu_args)

debug: build
	$(qemu) $(qemu_args) -s -S &
	sleep 1
	$(GDB) $(kernel_elf) -ex 'target remote localhost:1234'

clean:
	@rm -f `find $(KMODULE_DIR) ! -name README.md -type f`
	@cargo clean

clippy:
	cargo clippy --target $(target)

fmt:
	cargo fmt --all

test:
	cargo test --workspace --exclude "arceos-*" --exclude "libax_bindings" -- --nocapture

test_no_fail_fast:
	cargo test --workspace --exclude "arceos-*" --exclude "libax_bindings" --no-fail-fast -- --nocapture

bootrd: FORCE
	@rm -f $(KMODULE_DIR)/bootrd.disk $(KMODULE_DIR)/top_*.json
	@ ../tools/mk_bootrd/mk_bootrd

profiles:
	@ ../tools/ch_bootrd/ch_bootrd $(KMODULE_DIR)/bootrd.disk

.PHONY: build cargo_build kernel_map disasm run justrun clean \
	clippy fmt test test_no_fail_fast bootrd profiles components FORCE
