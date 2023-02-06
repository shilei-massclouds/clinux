/*
 * Module
 */

#ifndef _MODULE_H_
#define _MODULE_H_

#include <stdint.h>
#include "list.h"

typedef enum {
    M_STATUS_NONE = 0,
    M_STATUS_DOING,
    M_STATUS_DONE,
} mod_status;

typedef struct _module {
    char        *name;
    mod_status  status;

    list_head   list;

    list_head   undef_syms;

    list_head   dependencies;
    int num_dependencies;
    uint64_t    offset_in_file;   
} module;

typedef struct _depend {
    module      *mod;
    list_head   list;
} depend;

typedef struct _symbol {
    char        *name;
    module      *mod;

    list_head   list;
} symbol;

typedef void (*sort_callback)(module *mod,
                              int *num_profile_mods,
                              uint64_t *profile_mods);

/*
 * ELF64
 */

#define SHT_SYMTAB  2

#define SHN_UNDEF   0x0

#define ELFMAG      "\177ELF"
#define SELFMAG     4

#define EI_NIDENT   16

typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT]; /* ELF "magic number" */
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;          /* Entry point virtual address */
  uint64_t e_phoff;          /* Program header table file offset */
  uint64_t e_shoff;          /* Section header table file offset */
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_shdr {
  uint32_t sh_name;      /* Section name, index in string tbl */
  uint32_t sh_type;      /* Type of section */
  uint64_t sh_flags;     /* Miscellaneous section attributes */
  uint64_t sh_addr;      /* Section virtual addr at execution */
  uint64_t sh_offset;    /* Section file offset */
  uint64_t sh_size;      /* Size of section in bytes */
  uint32_t sh_link;      /* Index of another section */
  uint32_t sh_info;      /* Additional section information */
  uint64_t sh_addralign; /* Section alignment */
  uint64_t sh_entsize;   /* Entry size if section holds table */
} Elf64_Shdr;

typedef struct elf64_sym {
  uint32_t st_name;      /* Symbol name, index in string tbl */
  uint8_t  st_info;      /* Type and binding attributes */
  uint8_t  st_other;     /* No defined meaning, 0 */
  uint16_t st_shndx;     /* Associated section index */
  uint64_t st_value;     /* Value of the symbol */
  uint64_t st_size;      /* Associated symbol size */
} Elf64_Sym;

#endif /* _MODULE_H_ */
