riscv64-linux-gnu-nm -n vmlinux.o | grep "\( [BDR] \)" | awk -F " " '{print $3}'
