# cLinux
A re-implementation of Linux Kernel based on components.
My goal is to replace C-component with Rust-component one-by-one.

🚧 Working In Progress.

### Preparation
Install libjson to dump component-relationship chat.
```sh
sudo apt install libjson-c-dev
```
Build busybox as the base of rootfs.
```sh
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- menuconfig
mkdir ./output_riscv64
make -j6 ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- CONFIG_PREFIX=./output_riscv64 install
```

### Build & Run
```sh
make disk_img
make run
```
