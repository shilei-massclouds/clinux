riscv64-linux-gnu-nm -n $1 | grep "\( [U] \)" | awk -F " " '{print $2}'
