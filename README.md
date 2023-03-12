# cLinux
An experimental componentized Linux Kernel.
My goal is to build different types of OS based on component lib.

Now most of the components are extracted from Linux Kernel
and organized into a component lib.
In future, I plan to introduce components from other OS kernel.

Components can be written in C or Rust.

🚧 Working In Progress.

### Build & Run
##### Run environment
Put qemu & opensbi at the same level with clinux.
```mermaid
graph TD;
top_directory --> qemu
top_directory --> opensbi
top_directory --> clinux
```
##### OpenSBI
```sh
git clone https://github.com/riscv/opensbi.git --depth 1
make PLATFORM=generic CROSS_COMPILE=riscv64-linux-gnu- PLATFORM_RISCV_XLEN=64
```
##### Qemu
```sh
git clone https://git.qemu.org/git/qemu.git --depth 1
./configure --target-list=riscv64-softmmu
make
```
##### cLinux
```sh
make
make bootrd
make run
```
### Test
Now there're three profiles
```sh
$ make profiles
Bootrd filename: ./output/bootrd.disk

Bootrd profiles: total[3]:
Bootrd profiles: offset(39f418) curr[39f418]:
  -> [0]: arceos_hello[39f418]
     [1]: memory_addr[39f478]
     [2]: linux[39f4b8]
```
In default, this is a Unikernel-style OS (Components from ArceOS).
We can change the profile
```sh
$ tools/ch_bootrd/ch_bootrd ./output/bootrd.disk -s 2
Bootrd filename: ./output/bootrd.disk

Bootrd profiles: total[3]:
Bootrd profiles: offset(39f418) curr[39f418]:
     [0]: arceos_hello[39f418]
     [1]: memory_addr[39f478]
  -> [2]: linux[39f4b8]
```
And then, run again
```sh
$ ./scripts/qemu.sh

[...]

do_read_fault: addr(51000) pgoff(41) flags(254)
page_add_file_rmap: NOT implemented!
alloc_set_pte: addr(51000) entry(200D8C5B)
_ksys_write: 1 fd(1)
_ksys_write: 2
serial8250_tx_chars: send count(16) ...
[Hello, world!]
vfs_write: ret(17)
--- --- _do_page_fault: cause(D) addr(78200)
--- handle_pte_fault: vmf: addr(78000) flags(254) pgoff(78)
do_anonymous_page: addr(78000) pgoff(78) flags(254)
--- --- _do_page_fault: cause(F) addr(781F8)
--- handle_pte_fault: vmf: addr(78000) flags(255) pgoff(78)
--- wp_page_copy: 1.2
--- wp_page_copy: 3
_do_group_exit: ...

########################
PANIC: do_exit (sys/exit.c:12)
NOW user-space app exit! [0]
########################
cloud@server:~/gitStudy/clinux$
```
Now this is a Linux-kernel-like OS from boot to first user-process(init).
For experiemnt, this user-process init just prints [Hello, wolrd!] and exit.
