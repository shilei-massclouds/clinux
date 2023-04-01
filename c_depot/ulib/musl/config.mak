# This version of config.mak was generated by:
# ./configure --target=riscv64-linux-gnu
# Any changes made here will be lost if configure is re-run
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
ARCH = riscv64
SUBARCH = 
ASMSUBARCH = 
srcdir = .
prefix = /usr/local/musl
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(prefix)/lib
includedir = $(prefix)/include
syslibdir = /lib
CC = riscv64-linux-gnu-gcc
LD = riscv64-linux-gnu-ld
CFLAGS = 
CFLAGS_AUTO = -Os -pipe -fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -Wno-pointer-to-int-cast -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith -Werror=int-conversion -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=discarded-array-qualifiers -Waddress -Warray-bounds -Wchar-subscripts -Wduplicate-decl-specifier -Winit-self -Wreturn-type -Wsequence-point -Wstrict-aliasing -Wunused-function -Wunused-label -Wunused-variable
CFLAGS_C99FSE = -std=c99 -nostdinc -ffreestanding -fexcess-precision=standard -frounding-math -Wa,--noexecstack
CFLAGS_MEMOPS = -fno-tree-loop-distribute-patterns
CFLAGS_NOSSP = -fno-stack-protector
CPPFLAGS = 
LDFLAGS = 
LDFLAGS_AUTO = -Wl,--sort-section,alignment -Wl,--sort-common -Wl,--gc-sections -Wl,--hash-style=both -Wl,--no-undefined -Wl,--exclude-libs=ALL -Wl,--dynamic-list=./dynamic.list
CROSS_COMPILE = riscv64-linux-gnu-
LIBCC = -lgcc -lgcc_eh
OPTIMIZE_GLOBS = internal/*.c malloc/*.c string/*.c
ALL_TOOLS =  obj/musl-gcc
TOOL_LIBS =  lib/musl-gcc.specs
ADD_CFI = no
MALLOC_DIR = mallocng
WRAPCC_GCC = $(CC)
AOBJS = $(LOBJS)
