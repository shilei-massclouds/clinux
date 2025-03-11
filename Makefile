# SPDX-License-Identifier: GPL-2.0
ARCH ?= riscv64

export MAKE := @make --no-print-directory
export KMODULE_DIR = $(CURDIR)/target/_bootrd/

#TOP ?= linux
TOP ?= virtio_blk
export TOP_COMPONENT := top_$(TOP)

DEP ?= err

DEBUG ?= n
ifeq ($(DEBUG),y)
  DEBUG_INC := -DDEBUG
else
  DEBUG_INC :=
endif
export DEBUG_INC

FS_TYPE := ext2
DISK_IMG := disk.img

include ./scripts/Makefile.include
include ./scripts/Makefile.utils
include include/config/auto.conf

# QEMU
QEMU := qemu-system-$(ARCH)
QEMU_ARGS := -m 512M
QEMU_ARGS += \
	-machine virt \
	-bios default \
	-kernel target/kernel.bin \
	-device virtio-blk-device,drive=disk0 \
	-drive id=disk0,if=none,format=raw,file=$(DISK_IMG) \
	-device virtio-net-device,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::5555-:5555 \
	-nographic \
    -append "earlycon=sbi root=/dev/vda rw console=ttyS0" \
    -d in_asm,int -D /tmp/qemu.log

# All component subdir
components := \
	prebuilt booter lib lib_user math flex_proportions crc32 \
	radix_tree idr xarray assoc_array list_lru klist logic_pio \
	refcount ratelimit \
	kasprintf random crypto uuid rhashtable iov_iter checksum \
	sbitmap scatterlist bio mempool truncate interval_tree \
	traps irq softirq dec_and_lock mpage initcalls \
	kobject driver_base resource devres dynamic_queue_limits \
	of of_irq irq_work tty vt earlycon dummycon input \
	sbi time irqchip riscv_cpu timer_riscv calibrate timerqueue \
	early_fdt params module patch maccess capability security \
	clocksource pid seq_file errseq futex dnotify \
	cpu task cgroup ksysfs dma up notify fs_attr fs_stat fs_writeback \
	jump_label extable vdso utsname keys ucount umh locks \
	early_sched sched task_work itimer \
	percpu percpu_refcount workqueue_itf workqueue notifier workingset \
	memblock paging rmap backing_dev riscv_mm_context mprotect bootmem \
	page_alloc slub vmalloc mm_util vmscan ioremap gup \
	spinlock semaphore mutex rwsem percpu_rwsem rcu lockref rtmutex \
	ipc ipc_sem ipc_shm watchdog noinitramfs oom_kill \
	fs inode dcache d_path file_table do_mounts file \
	open readahead async xattr user readdir \
	cred groups posix_acl user_namespace nsproxy fs_namespace pid_namespace \
	ramfs kernfs sysfs proc block_dev char_dev filesystems shmem nsfs devtmpfs \
	fs_context fs_parser fs_struct namei libfs debugfs \
	ipc_namespace drv_char 8250 serial drv_clk \
	net_core net_core_dev netlink string_helpers net_namespace net_sched \
	ipv4 tcp tcp_ipv4 udp udplite icmp ping arp ip_input ip_output \
	devinet fib tcp_metrics tcp_cong ipv4_proc ip_tunnel \
	datagram loopback ip_sockglue filter skbuff socket \
	tcp_timer inet_connection_sock tcp_ulp tcp_fastopen \
	stream dst ethernet dev_addr_lists xdp ethtool link_watch \
	block genhd partitions elevator sysctl_net inet_hashtables tcp_output \
	fork mmap filemap kthread riscv_process exit exec \
	buffer swap riscv_fault aio signalfd \
	signal sysctl kallsyms read_write \
	virtio_mmio virtio virtio_blk virtio_net ext2 binfmt_elf \
	sys ioctl fcntl nlattr \
	early_printk printk panic bug dump_stack show_mem

SELECTED = $(shell cat $(KMODULE_DIR)selected.in)
CL_INIT := $(KMODULE_DIR)cl_init

all: build

build: preclean predirs tools vdso_payload target/kernel.bin

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

necessities: $(components) top_component
	@ RUST_LOG=$(DEP) ./tools/find_dep/target/release/find_dep $(KMODULE_DIR) $(TOP_COMPONENT)

tools:
	$(MAKE) -C ./tools

$(components): FORCE
	@mkdir -p ./target/$@
	$(MAKE) -f ./scripts/Makefile.build obj=$@

top_component: FORCE
	@rm -rf ./target/top_component
	@mkdir -p ./target/top_component
	$(MAKE) -f ./scripts/Makefile.build obj=top_component
	@mv $(KMODULE_DIR)top_component.ko $(KMODULE_DIR)$(TOP_COMPONENT).ko

predirs:
	@mkdir -p ./target/_bootrd

vdso_payload:
	@mkdir -p ./target/$@
	$(MAKE) -f ./vdso_payload/Makefile obj=$@

run: build
ifeq ($(wildcard $(DISK_IMG)),)
	@printf "warning: image \"$(DISK_IMG)\" needs to be built first!\n"
	@exit 1
endif
	$(QEMU) $(QEMU_ARGS)

clean:
	@rm -rf ./target
	$(MAKE) -C ./tools clean

preclean:
	@rm -f $(CL_INIT).c

linux_apps:
	@make -C $@

disk_img: linux_apps
	rm -f $(DISK_IMG)
	$(call make_disk_image,$(FS_TYPE),$(DISK_IMG))
	$(call $(ARCH)_install_apps,$(DISK_IMG))

FORCE:

.PHONY: all build tools necessities components predirs vdso_payload clean top_component disk_img linux_apps FORCE
