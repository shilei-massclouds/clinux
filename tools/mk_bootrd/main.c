// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <json.h>

#include "module.h"
#include "../../include/config.h"
#include "../../include/bootrd.h"

/* n must be power of 2 */
#define ROUND_UP(x, n) (((x) + (n) - 1UL) & ~((n) - 1UL))

#define KMODULE_DIR "../../output/"
#define BOOTRD_FILENAME KMODULE_DIR "bootrd.disk"

/* Bootrd file is used as qemu.pflash */
#define BOOTRD_SIZE 32*1024*1024L   /* 32M */

#define DEFAULT_PROFILE "./profile.json"
static json_object *profile_top = NULL;

static uint32_t ksym_ptr;
static uint32_t ksym_num;

static int num_modules = 0;
static LIST_HEAD(modules);
static LIST_HEAD(symbols);

static int num_profile_mods = 0;
static uint64_t *profile_mods = NULL;

static bool
check_module(const char *name)
{
    char *p;

    if (!strncmp(name, "test_", 5))
        return false;

    p = strrchr(name, '.');
    return !strcmp(p, ".ko");
}

static void
discover_modules(void)
{
    int n;
    struct dirent **namelist;
    bool has_startup = false;
    bool has_system_map = false;

    n = scandir(KMODULE_DIR, &namelist, NULL, NULL);
    if (n == -1) {
        printf("%s: error when scandir!\n", __func__);
        exit(-1);
    }

    while (n--) {
        if (check_module(namelist[n]->d_name)) {
            module *mod = calloc(1, sizeof(module));
            mod->name = strdup(namelist[n]->d_name);
            printf("%s: \n", mod->name);
            INIT_LIST_HEAD(&(mod->undef_syms));
            INIT_LIST_HEAD(&(mod->dependencies));
            list_add_tail(&(mod->list), &modules);
            num_modules++;
        } else if (!strcmp(namelist[n]->d_name, "startup.bin")) {
            has_startup = true;
        } else if (!strcmp(namelist[n]->d_name, "System.map")) {
            has_system_map = true;
        }

        free(namelist[n]);
    }
    free(namelist);

    if (!has_startup) {
        printf("%s: No startup.bin\n", __func__);
        exit(-1);
    }

    if (!has_system_map) {
        printf("%s: No System.map\n", __func__);
        exit(-1);
    }
}

static void
detect_syms_range(void)
{
    FILE *fp;
    char line[256];
    uint64_t start = 0;
    uint64_t end = 0;

    fp = fopen(KMODULE_DIR "System.map", "r");
    if (fp == NULL) {
        printf("No System.map\n");
        exit(-1);
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "_start_ksymtab_strings")) {
            char *tail = strchr(line, ' ');
            start = strtoul(line, &tail, 16);
        } else if (strstr(line, "_end_ksymtab_strings")) {
            char *tail = strchr(line, ' ');
            end = strtoul(line, &tail, 16);
        }

        if (start && end)
            break;
    }

    fclose(fp);

    /* Address to offset within the module */
    ksym_ptr = (uint32_t)(start - CONFIG_PAGE_OFFSET);
    ksym_num = (uint32_t)(end - start);
}

static void
export_symbols(const char *start, const char *end,
               list_head *sym_list, module *mod)
{
    while (start < end) {
        symbol *sym = calloc(1, sizeof(symbol));
        sym->name = strdup(start);
        sym->mod = mod;
        list_add_tail(&(sym->list), sym_list);

        printf("%s: export(%s)\n", __func__, start);
        start = strchr(start, '\0');
        start++;
    }
}

static void
init_symtable(void)
{
    FILE *fp;
    char *buf;
    char *cur;

    detect_syms_range();
    printf("range(0x%x, 0x%x)\n", ksym_ptr, ksym_num);

    fp = fopen(KMODULE_DIR "startup.bin", "rb");
    if (fp == NULL) {
        printf("No System.map\n");
        exit(-1);
    }

    fseek(fp, (long)ksym_ptr, SEEK_SET);

    buf = calloc(ksym_num, 1);
    fread(buf, 1, ksym_num, fp);

    export_symbols(buf, buf + ksym_num, &symbols, NULL);

    free(buf);
    fclose(fp);
}

static char *
get_strtab(Elf64_Shdr *shdr, FILE *fp)
{
    char *str;

    str = (char *) calloc(shdr->sh_size, 1);
    printf("%s: offset(%lx)\n", __func__, shdr->sh_offset);

    fseek(fp, (long)shdr->sh_offset, SEEK_SET);
    if (fread(str, 1, shdr->sh_size, fp) != shdr->sh_size) {
        printf("Read str section error\n");
        exit(-1);
    }

    return str;
}

static void
discover_undef_syms(module *mod,
                    Elf64_Shdr *shdr,
                    const char *strtab,
                    FILE *fp)
{
    int i;
    Elf64_Sym *sym;

    sym = (Elf64_Sym *)calloc(shdr->sh_size, 1);

    fseek(fp, (long)shdr->sh_offset, SEEK_SET);
    if (fread(sym, 1, shdr->sh_size, fp) != shdr->sh_size) {
        printf("Bad sym section\n");
        exit(-1);
    }

    for (i = 1; i < shdr->sh_size / sizeof(Elf64_Sym); i++) {
        if (sym[i].st_shndx == SHN_UNDEF) {
            symbol *undef = calloc(1, sizeof(symbol));
            undef->name = strdup(strtab + sym[i].st_name);
            list_add_tail(&(undef->list), &(mod->undef_syms));
            printf("%s: name(%s)\n", __func__, undef->name);
        }
    }

    free(sym);
}

static symbol *
match_undef(const char *name)
{
    symbol *sym;

    list_for_each_entry(sym, &(symbols), list) {
        if (strcmp(sym->name, name) == 0)
            return sym;
    }

    return NULL;
}

static bool
find_dependency(module *cur, module *target)
{
    depend *dep;

    list_for_each_entry(dep, &(cur->dependencies), list) {
        if (dep->mod == target)
            return true;
    }

    return false;
}

static void
build_dependency(module *mod)
{
    list_head *p, *n;

    list_for_each_safe(p, n, &(mod->undef_syms)) {
        symbol *undef = list_entry(p, symbol, list);
        symbol *sym = match_undef(undef->name);
        if (sym == NULL) {
            printf("mod '%s': undef sym '%s' cannot be resolved!\n",
                   mod->name, undef->name);
            exit(-1);
        }

        if (sym->mod && !find_dependency(mod, sym->mod)) {
            depend *d = calloc(1, sizeof(depend));
            d->mod = sym->mod;
            list_add_tail(&(d->list), &(mod->dependencies));
        }

        list_del(&(undef->list));
        free(undef);
    }

    if (!list_empty(&(mod->undef_syms))) {
        printf("mod '%s' still has undef symbols!\n", mod->name);
        exit(-1);
    }
}

static void
analysis_module(module *mod)
{
    int i;
    FILE *fp;
    Elf64_Shdr *sechdrs;
    char *secstrings;
    Elf64_Ehdr hdr = {0};
    char filename[256] = {0};

    printf("%s: %s\n", __func__, mod->name);
    sprintf(filename, "%s%s", KMODULE_DIR, mod->name);
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("bad file (%s)\n", filename);
        exit(-1);
    }

    if (fread(&hdr, sizeof(Elf64_Ehdr), 1, fp) != 1) {
        printf("Read elf header error\n");
        exit(-1);
    }

    fseek(fp, (long)hdr.e_shoff, SEEK_SET);

    sechdrs = calloc(hdr.e_shnum, sizeof(Elf64_Shdr));

    if (fread(sechdrs, sizeof(Elf64_Shdr), hdr.e_shnum, fp) != hdr.e_shnum) {
        printf("Read section header error\n");
        exit(-1);
    }

    secstrings = get_strtab(&sechdrs[hdr.e_shstrndx], fp);

    for (i = 0; i < hdr.e_shnum; i++) {
        Elf64_Shdr *shdr = sechdrs + i;

        if (shdr->sh_type == SHT_SYMTAB) {
            char *strtab;
            strtab = get_strtab(&sechdrs[shdr->sh_link], fp);

            printf("[%d]: SHT_SYMTAB (%lx, %lx)\n",
                   i, shdr->sh_offset, shdr->sh_size);

            /* Discover all undef syms for this mod */
            discover_undef_syms(mod, shdr, strtab, fp);
            free(strtab);
        } else {
            const char *name = secstrings + shdr->sh_name;
            if (strcmp(name, "_ksymtab_strings") == 0) {
                char *ksym_str = get_strtab(shdr, fp);
                export_symbols(ksym_str, ksym_str + shdr->sh_size,
                               &symbols, mod);
            }
        }
    }

    free(secstrings);
    free(sechdrs);
    fclose(fp);
}

static void
traverse_dependency(module *mod, sort_callback cb, void *opaque)
{
    list_head *p, *n;

    //printf("### %s: '%s' (%d)\n", __func__, mod->name, mod->status);

    if (mod->status)
        return;

    mod->status = M_STATUS_DOING;

    json_object *dep_mods = json_object_new_array();

    list_for_each_safe(p, n, &mod->dependencies) {
        depend *d = list_entry(p, depend, list);

        //printf("%s: '%s' -> '%s'\n", __func__, mod->name, d->mod->name);

        traverse_dependency(d->mod, cb, opaque);
        if (d->mod->status != M_STATUS_DONE) {
            printf("%s: cyclic chain: '%s'\n", __func__, d->mod->name);
            return;
        }

        json_object_array_add(dep_mods,
                              json_object_new_string(d->mod->name));

        list_del(&(d->list));
        free(d);
    }

    json_object_object_add(profile_top, mod->name, dep_mods);

    if (!list_empty(&(mod->dependencies))) {
        printf("'%s' broken dependencies!\n", mod->name);
        exit(-1);
    }

    mod->status = M_STATUS_DONE;
    cb(mod->name, opaque);
}

static void
write_profile(struct bootrd_header *hdr, void *opaque)
{
    uint64_t ret;
    FILE *fp = (FILE *) opaque;

    struct profile_header header;
    memcpy(&header.magic, &PROFILE_MAGIC, sizeof(header.magic));
    header.version = 1;
    header.total_size = sizeof(struct profile_header) +
        sizeof(uint64_t) * num_profile_mods;
    header.mod_num = num_profile_mods;

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        printf("%s: cannot write profile header to bootrd file!\n",
               __func__);
        exit(-1);
    }
    printf("###### %s: num_profile_mods (%d)\n", __func__, num_profile_mods);
#if 0
    {
        int i = 0;
        for (i = 0; i < num_profile_mods; i++) {
            printf("###### %s: (%d)\n", __func__, profile_mods[i]);
        }
    }
#endif
    ret = fwrite(profile_mods, sizeof(uint64_t), num_profile_mods, fp);
    if (ret != num_profile_mods) {
        printf("%s: cannot profile data into bootrd file!\n", __func__);
        exit(-1);
    }

    hdr->profile_num++;
}

void
sort_modules(struct bootrd_header *hdr, sort_callback cb, void *opaque)
{
    list_head *p, *n;
    module *mod;

    discover_modules();
    hdr->mod_num = num_modules;

    /* Init ksymtab based on startup.bin. */
    init_symtable();

    /* For each module, register its exported symbols into a list */
    list_for_each_entry(mod, &modules, list)
        analysis_module(mod);

    /*
     * For each module, try to match its undef symbols with
     * those in the above symbol list.
     * By this way, we can build dependency chains among these modules.
     */
    list_for_each_entry(mod, &modules, list)
        build_dependency(mod);

    profile_mods = calloc(num_modules, sizeof(uint64_t));

    /* Traverse modules based on their dependency chains,
     * try to build bootrd and profiles */
    list_for_each_entry(mod, &modules, list) {
        traverse_dependency(mod, cb, opaque);
        if (mod->status != M_STATUS_DONE) {
            printf("%s: cyclic chain: '%s'.\n", __func__, mod->name);
            exit(-1);
        }
    }

    /* Create module profile(json-style) for human */
    json_object_to_file(DEFAULT_PROFILE, profile_top);

    /* Add module profiles into bootrd */
    hdr->profile_offset = ftell((FILE *) opaque);
    hdr->current_profile = hdr->profile_offset;
    write_profile(hdr, opaque);

    free(profile_mods);
    profile_mods = NULL;
    num_profile_mods = 0;
}

void
clear_symbols(void)
{
    list_head *p, *n;
    symbol *sym;

    list_for_each_safe(p, n, &symbols) {
        sym = list_entry(p, symbol, list);
        list_del(&(sym->list));
        free(sym->name);
        free(sym);
    }
}

void
clear_modules(void)
{
    list_head *p, *n;
    module *mod;

    list_for_each_safe(p, n, &modules) {
        mod = list_entry(p, module, list);
        list_del(&(mod->list));
        free(mod->name);
        free(mod);
    }
}

/* Since e_phoff makes no sense, use it to save module length. */
static inline void
elf64_hdr_set_length(void *ptr, uint64_t length)
{
    if (memcmp(ptr, ELFMAG, SELFMAG)) {
        printf("%s: bad elf64 header\n", __func__);
        exit(-1);
    }
    struct elf64_hdr *hdr = (struct elf64_hdr *)ptr;
    hdr->e_phoff = length;
}

static uint8_t *
read_module(const char *filename, long *psize)
{
    struct stat info;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL || fstat(fileno(fp), &info) < 0) {
        printf("%s: bad filename %s\n", __func__, filename);
        exit(-1);
    }

    *psize = ROUND_UP((size_t) info.st_size, 8);

    uint8_t *data = calloc(*psize, 1);
    if (data == NULL) {
        printf("%s: alloc memory failed!\n", __func__);
        exit(-1);
    }
    if (fread(data, 1, (size_t) info.st_size, fp) != info.st_size) {
        printf("%s: read file failed!\n", __func__);
        exit(-1);
    }

    /* HACK!!! */
    elf64_hdr_set_length(data, (uint64_t)info.st_size);
    fclose(fp);

    return data;
}

static void
sort_func(const char *name, void *opaque)
{
    size_t size = 0;
    char filename[256] = {0};
    sprintf(filename, "%s%s", KMODULE_DIR, name);
    uint8_t *data = read_module(filename, &size);
    if (data == NULL || size == 0) {
        printf("cannont read module '%s'!\n", filename);
        exit(-1);
    }

    FILE *fp = (FILE *) opaque;
    uint64_t offset = ftell(fp);
    if (fwrite(data, 1, size, fp) != size) {
        printf("%s: cannot module data into bootrd file!\n", __func__);
        exit(-1);
    }
    free(data);
    data = NULL;

    if (num_profile_mods == num_modules) {
        printf("%s: find too many modules %d, limit is %d!\n",
               __func__, num_profile_mods, num_modules);
        exit(-1);
    }
    profile_mods[num_profile_mods++] = offset;
#if 0
    printf("%s: mod '%s'; size '%ld'; offset '%ld'\n",
           __func__, name, size, offset);
#endif
}

FILE *
create_bootrd(struct bootrd_header *hdr)
{
    memcpy(&(hdr->magic), &BOOTRD_MAGIC, sizeof(hdr->magic));
    hdr->version = 1;
    hdr->mod_offset = sizeof(*hdr);

    FILE *fp = fopen(BOOTRD_FILENAME, "rb+");
    if (fp == NULL) {
        printf("%s: no base bootrd file!\n", __func__);
        exit(-1);
    }
    fseek(fp, sizeof(*hdr), SEEK_SET);
    return fp;
}

void
complete_bootrd(struct bootrd_header *hdr, FILE *fp)
{
    hdr->total_size = ftell(fp);

#if 0
    printf("%s: magic %x\n", __func__, hdr->magic);
    printf("%s: version %x\n", __func__, hdr->version);
    printf("%s: total_size %x\n", __func__, hdr->total_size);
    printf("%s: mod_offset %x\n", __func__, hdr->mod_offset);
    printf("%s: mod_num %x\n", __func__, hdr->mod_num);
    printf("%s: profile_offset %x\n", __func__, hdr->profile_offset);
    printf("%s: profile_num %x\n", __func__, hdr->profile_num);
    printf("%s: current_profile %x\n", __func__, hdr->current_profile);
#endif

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) != BOOTRD_SIZE) {
        printf("%s: bad bootrd file (%ld != 32M).\n",
               __func__, ftell(fp));
        exit(-1);
    }

    /* overwrite bootrd header */
    fseek(fp, 0, SEEK_SET);
    if (fwrite(hdr, sizeof(*hdr), 1, fp) != 1) {
        printf("%s: cannot overwrite bootrd header!\n", __func__);
        exit(-1);
    }

    fclose(fp);
}

int
main(void)
{
    struct bootrd_header hdr;
    memset(&hdr, 0, sizeof(hdr));

    if (!profile_top)
        profile_top = json_object_new_object();

    FILE* fp = create_bootrd(&hdr);
    sort_modules(&hdr, sort_func, fp);
    complete_bootrd(&hdr, fp);
    clear_symbols();
    clear_modules();

    if (profile_top) {
        json_object_put(profile_top);
        profile_top = NULL;
    }

    return 0;
}
