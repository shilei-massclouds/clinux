/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_ELF_H
#define _UAPI_LINUX_ELF_H

#include <types.h>

/* sh_flags */
#define SHF_WRITE           0x1
#define SHF_ALLOC           0x2
#define SHF_EXECINSTR       0x4
#define SHF_RELA_LIVEPATCH  0x00100000
#define SHF_RO_AFTER_INIT   0x00200000
#define SHF_MASKPROC        0xf0000000

/* sh_type */
#define SHT_NULL    0
#define SHT_PROGBITS    1
#define SHT_SYMTAB  2
#define SHT_STRTAB  3
#define SHT_RELA    4
#define SHT_HASH    5
#define SHT_DYNAMIC 6
#define SHT_NOTE    7
#define SHT_NOBITS  8
#define SHT_REL     9
#define SHT_SHLIB   10
#define SHT_DYNSYM  11
#define SHT_NUM     12
#define SHT_LOPROC  0x70000000
#define SHT_HIPROC  0x7fffffff
#define SHT_LOUSER  0x80000000
#define SHT_HIUSER  0xffffffff

/* special section indexes */
#define SHN_UNDEF       0x0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_LIVEPATCH   0xff20
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff

#define ELF64_R_SYM(i)  ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)

#define ELFMAG      "\177ELF"
#define SELFMAG     4

#define EI_NIDENT   16

/* Relocation types used by the dynamic linker */
#define R_RISCV_NONE            0
#define R_RISCV_32              1
#define R_RISCV_64              2
#define R_RISCV_RELATIVE        3
#define R_RISCV_COPY            4
#define R_RISCV_JUMP_SLOT       5
#define R_RISCV_TLS_DTPMOD32    6
#define R_RISCV_TLS_DTPMOD64    7
#define R_RISCV_TLS_DTPREL32    8
#define R_RISCV_TLS_DTPREL64    9
#define R_RISCV_TLS_TPREL32     10
#define R_RISCV_TLS_TPREL64     11

/* Relocation types not used by the dynamic linker */
#define R_RISCV_BRANCH          16
#define R_RISCV_JAL             17
#define R_RISCV_CALL            18
#define R_RISCV_GOT_HI20        20
#define R_RISCV_TLS_GOT_HI20    21
#define R_RISCV_TLS_GD_HI20     22
#define R_RISCV_PCREL_HI20      23
#define R_RISCV_PCREL_LO12_I    24
#define R_RISCV_PCREL_LO12_S    25

#define R_RISCV_ADD32           35
#define R_RISCV_SUB32           39

#define R_RISCV_RVC_BRANCH      44
#define R_RISCV_RVC_JUMP        45
#define R_RISCV_RELAX           51

/* These constants define the different elf file types */
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define PT_LOAD     1
#define PT_INTERP   3

#define PT_LOOS             0x60000000      /* OS-specific */
#define PT_GNU_PROPERTY     0x6474e553
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

#define PT_GNU_STACK    (PT_LOOS + 0x474e551)

#define EM_RISCV    243 /* RISC-V */

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) ((x)->e_machine == EM_RISCV)

#define elfhdr      elf64_hdr
#define elf_phdr    elf64_phdr

typedef u64   Elf64_Off;
#define elf_addr_t  Elf64_Off

#define ELF_EXEC_PAGESIZE   (PAGE_SIZE)

/*
 * This is the location that an ET_DYN program is loaded if exec'ed.  Typical
 * use of this is to invoke "./ld.so someprog" to test out a new version of
 * the loader.  We need to make sure that it is out of the way of the program
 * that it will "exec", and that there is sufficient room for the brk.
 */
#define ELF_ET_DYN_BASE     ((TASK_SIZE / 3) * 2)

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R        0x4
#define PF_W        0x2
#define PF_X        0x1

typedef struct elf64_phdr {
    u32 p_type;
    u32 p_flags;
    u64 p_offset;   /* Segment file offset */
    u64 p_vaddr;    /* Segment virtual address */
    u64 p_paddr;    /* Segment physical address */
    u64 p_filesz;   /* Segment size in file */
    u64 p_memsz;    /* Segment size in memory */
    u64 p_align;    /* Segment alignment, file & memory */
} Elf64_Phdr;

typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u64 e_entry;          /* Entry point virtual address */
  u64 e_phoff;          /* Program header table file offset */
  u64 e_shoff;          /* Section header table file offset */
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_shdr {
  u32 sh_name;      /* Section name, index in string tbl */
  u32 sh_type;      /* Type of section */
  u64 sh_flags;     /* Miscellaneous section attributes */
  u64 sh_addr;      /* Section virtual addr at execution */
  u64 sh_offset;    /* Section file offset */
  u64 sh_size;      /* Size of section in bytes */
  u32 sh_link;      /* Index of another section */
  u32 sh_info;      /* Additional section information */
  u64 sh_addralign; /* Section alignment */
  u64 sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

typedef struct elf64_sym {
  u32 st_name;      /* Symbol name, index in string tbl */
  u8  st_info;      /* Type and binding attributes */
  u8  st_other;     /* No defined meaning, 0 */
  u16 st_shndx;     /* Associated section index */
  u64 st_value;     /* Value of the symbol */
  u64 st_size;      /* Associated symbol size */
} Elf64_Sym;

typedef struct elf64_rela {
  u64 r_offset; /* Location at which to apply the action */
  u64 r_info;   /* index and type of relocation */
  s64 r_addend; /* Constant addend used to compute value */
} Elf64_Rela;

#endif /* _UAPI_LINUX_ELF_H */
