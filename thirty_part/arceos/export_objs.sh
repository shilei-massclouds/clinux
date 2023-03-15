#!/bin/sh

SRC_DIR=./target/riscv64gc-unknown-none-elf/release/deps/
DST_DIR=../clinux/extern_rlib/

for com in axhal axlog axruntime axalloc \
    compiler_builtins core alloc allocator \
    page_table_entry page_table \
    buddy_system_allocator bitmap_allocator \
    libax log memory_addr riscv arceos_helloworld arceos_memtest ;
do
    rm -rf $DST_DIR/$com
    mkdir -p $DST_DIR/$com
    cp $SRC_DIR/lib$com-*.rlib $DST_DIR/$com
    cd $DST_DIR/$com
    ar -x ./lib$com-*.rlib
    pwd
    cd -
done

