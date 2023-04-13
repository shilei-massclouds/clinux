#!/bin/sh

./clr.sh

dd if=/dev/zero of=./rootfs.ext2 bs=1M count=32
mkfs.ext2 -b 1024 -I 128 ./rootfs.ext2
mkdir fs
sudo mount -o loop rootfs.ext2 ./fs/
sudo mkdir fs/sbin
sudo mkdir fs/etc
sudo mkdir fs/lib
#riscv64-linux-gnu-gcc -o ./init init.c
#riscv64-linux-gnu-gcc -static -o ./init init.c
~/study/musl-1.2.3/output/bin/musl-gcc -I. -I/home/cloud/study/musl-1.2.3/output/include -static -o ./init init.c -L /home/cloud/study/musl-1.2.3/output/lib -l:libc.a #-l:libgcc.a
sudo cp ./init ./fs/sbin/
#sudo cp /usr/riscv64-linux-gnu/lib/ld-linux-riscv64-lp64d.so.1 ./fs/lib
#sudo cp /usr/riscv64-linux-gnu/lib/libc.so.6 ./fs/lib
sudo umount ./fs
mkdir ~/gitStudy/qemu/image/ 2>/dev/null
cp ./rootfs.ext2 /tmp/disk.img
