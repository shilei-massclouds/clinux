riscv64-linux-gnu-nm -n $1 | grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)\|\( \.L\)' | awk -F " " '{print $3}'
