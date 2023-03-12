#!/bin/sh

SRC_DIR=./target/riscv64gc-unknown-none-elf/release/deps/
DST_DIR=../clinux/extern_rlib/

for com in axhal axlog axruntime compiler_builtins core \
    libax log memory_addr riscv arceos_helloworld ;
do
    mkdir -p $DST_DIR/$com
    cp $SRC_DIR/lib$com-*.rlib $DST_DIR/$com
    cd $DST_DIR/$com
    rm -f *.o
    ar -x ./lib$com-*.rlib
    pwd
    cd -
done

