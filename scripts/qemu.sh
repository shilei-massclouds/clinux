#
# Enable debug log
# -d in_asm -D /tmp/insturction.log
#

WORK_BASE=~/gitStudy

rm -f /tmp/insturction.log

${WORK_BASE}/qemu/build/qemu-system-riscv64 \
    -machine virt -cpu rv64 -m 2G \
    -bios ${WORK_BASE}/opensbi/build/platform/generic/firmware/fw_jump.bin \
    -kernel ${WORK_BASE}/clinux/output/startup.bin \
    -drive if=pflash,file=${WORK_BASE}/clinux/output/bootrd.disk,format=raw,unit=1 \
    -device virtio-blk-device,drive=hd1 -drive file=./tools/mk_rootfs/rootfs.ext2,if=none,id=hd1,format=raw \
    -device virtio-net-device,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:5555 \
    -object rng-random,filename=/dev/urandom,id=rng -device virtio-rng-device,rng=rng \
    -nographic -append "earlycon=sbi root=/dev/vda rw console=ttyS0" \
    -d in_asm -D /tmp/insturction.log
