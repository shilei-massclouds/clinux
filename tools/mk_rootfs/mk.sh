#!/bin/sh

./clr.sh

dd if=/dev/zero of=./rootfs.ext2 bs=1M count=32
mkfs.ext2 ./rootfs.ext2
mkdir fs
sudo mount -o loop rootfs.ext2 ./fs/
sudo mkdir fs/sbin
sudo mkdir fs/etc
sudo mkdir fs/lib
riscv64-linux-gnu-gcc -o ./init init.c
sudo cp ./init ./fs/sbin/
sudo cp /usr/riscv64-linux-gnu/lib/ld-linux-riscv64-lp64d.so.1 ./fs/lib
sudo cp /usr/riscv64-linux-gnu/lib/libc.so.6 ./fs/lib
sudo umount ./fs
cp ./rootfs.ext2 ~/gitStudy/qemu/image/test.raw
