// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <json.h>
#include <assert.h>

#include "module.h"
#include "../../include/config.h"
#include "../../include/bootrd.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))

/* n must be power of 2 */
#define ROUND_UP(x, n) (((x) + (n) - 1UL) & ~((n) - 1UL))

#define BOOTRD_FILENAME "bootrd.disk"

/* Bootrd file is used as qemu.pflash */
#define BOOTRD_SIZE 32*1024*1024L   /* 32M */

/* The main output directory for modules */
char *kmod_dir = NULL;

typedef depend* (*match_callback)(module *mod, symbol *sym);
static bool need_handle_candidates = false;

static module mod_startup;

static uint32_t ksym_ptr;
static uint32_t ksym_num;

static int num_modules = 0;
static LIST_HEAD(modules);
static LIST_HEAD(symbols);
static LIST_HEAD(fixups);

static uint8_t *
read_module(const char *filename, long *psize);

static void
clean_bootrd()
{
    if (kmod_dir == NULL)
        return;

    char filename[256] = {0};
    sprintf(filename, "%s/%s", kmod_dir, BOOTRD_FILENAME);
    remove(filename);
}

static void
terminate()
{
    clean_bootrd();
    exit(-1);
}

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
write_mod_to_bootrd(module *mod, FILE *fp)
{
    size_t size = 0;

    char filename[256] = {0};
    sprintf(filename, "%s/%s", kmod_dir, mod->name);
    uint8_t *data = read_module(filename, &size);
    if (data == NULL || size == 0) {
        printf("cannont read module '%s'!\n", filename);
        terminate();
    }

    mod->offset_in_file = ftell(fp);
    if (fwrite(data, 1, size, fp) != size) {
        printf("%s: cannot module data into bootrd file!\n", __func__);
        exit(-1);
    }
    free(data);
    data = NULL;
}

static void
discover_modules(FILE *fp)
{
    int n;
    struct dirent **namelist;
    bool has_startup = false;
    bool has_system_map = false;

    n = scandir(kmod_dir, &namelist, NULL, NULL);
    if (n == -1) {
        printf("%s: error when scandir!\n", __func__);
        exit(-1);
    }

    while (n--) {
        if (check_module(namelist[n]->d_name)) {
            module *mod = calloc(1, sizeof(module));
            mod->name = strdup(namelist[n]->d_name);
            //printf("%s: \n", mod->name);
            INIT_LIST_HEAD(&(mod->undef_syms));
            INIT_LIST_HEAD(&(mod->dependencies));
            mod->num_dependencies = 0;
            list_add_tail(&(mod->list), &modules);
            num_modules++;

            /* write into bootrd disk */
            write_mod_to_bootrd(mod, fp);
        } else if (!strcmp(namelist[n]->d_name, "startup.bin")) {
            has_startup = true;
        } else if (!strcmp(namelist[n]->d_name, "startup.map")) {
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
        printf("%s: No startup.map\n", __func__);
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

    char filename[256] = {0};
    sprintf(filename, "%s/startup.map", kmod_dir);
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("No startup.map\n");
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
    if (mod == NULL) {
        printf("%s: no module\n", __func__);
        exit(-1);
    }

    while (start < end) {
        symbol *sym = calloc(1, sizeof(symbol));
        sym->name = strdup(start);
        sym->mod = mod;
        list_add_tail(&(sym->list), sym_list);

        //printf("%s: export(%s)\n", __func__, start);
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

    char filename[256] = {0};
    sprintf(filename, "%s/startup.bin", kmod_dir);
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("No startup.bin\n");
        exit(-1);
    }

    fseek(fp, (long)ksym_ptr, SEEK_SET);

    buf = calloc(ksym_num, 1);
    fread(buf, 1, ksym_num, fp);

    export_symbols(buf, buf + ksym_num, &symbols, &mod_startup);

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
            //printf("%s: name(%s)\n", __func__, undef->name);
        }
    }

    free(sym);
}

static depend *
find_dependency(module *cur, module *target)
{
    depend *dep;

    list_for_each_entry(dep, &(cur->dependencies), list) {
        if (dep->mod == target)
            return dep;
    }

    return NULL;
}

static depend *
on_match(module *mod, symbol *sym)
{
    assert(sym->mod != NULL);
    printf("%s: mod '%s' sym '%s'\n", __func__, mod->name, sym->name);

    depend *d = find_dependency(mod, sym->mod);
    if (d == NULL) {
        d = calloc(1, sizeof(depend));
        d->mod = sym->mod;
        list_add_tail(&(d->list), &(mod->dependencies));
        mod->num_dependencies++;
        if (strcmp(mod->name, "rs_lib") == 0)
        printf("++++++ %s: mod '%s' -> '%s' sym '%s'\n",
               __func__, mod->name, sym->mod->name, sym->name);
    }
    return d;
}

static void
match_undef(const char *name, match_callback cb, module *mod)
{
    symbol *sym;

    depend *head = NULL;
    list_for_each_entry(sym, &(symbols), list) {
        if (strcmp(sym->name, name) == 0) {
            depend *cur = cb(mod, sym);
            assert(cur);

            if (head == NULL) {
                head = cur;
            } else {
                cur->next = head;
                head = cur;
            }
        }
    }

    if (head == NULL) {
        printf("mod '%s': undef sym '%s' cannot be resolved!\n",
               mod->name, name);
        terminate();
    }

    if (head->next == NULL)
        return;

    /* Okay, now there're more than one module that can solve
     * this undef symbol. Report this in the profile and require
     * manual intervention. */
    depend *p = head;
    while (p) {
        printf("%s: mod '%s'\n", __func__, p->mod->name);
        p->flags |= DEPEND_FLAGS_CANDIDATE;
        p = p->next;
    }

    depend *fixup = calloc(1, sizeof(depend));
    fixup->next = head;
    list_add_tail(&(fixup->list), &fixups);
}

static void
build_dependency(module *mod)
{
    list_head *p, *n;

    //printf("%s: mod '%s'\n", __func__, mod->name);
    list_for_each_safe(p, n, &(mod->undef_syms)) {
        symbol *undef = list_entry(p, symbol, list);
        match_undef(undef->name, on_match, mod);
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
    sprintf(filename, "%s/%s", kmod_dir, mod->name);
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
#if 1
                export_symbols(ksym_str, ksym_str + shdr->sh_size,
                               &symbols, mod);
#endif
            }
        }
    }

    free(secstrings);
    free(sechdrs);
    fclose(fp);
}

static bool
verify_dependency(const char *name, const char *dep_name,
                  json_object *json_top)
{
    //printf("%s: verify '%s' depend '%s'\n", __func__, name, dep_name);

    /* Of course, module 'startup' always exists. */
    if (strncmp(dep_name, "startup", strlen("startup")) == 0)
        return true;

    struct json_object *depend_obj = NULL;
    json_object_object_get_ex(json_top, "dependencies", &depend_obj);
    assert(depend_obj != NULL);

    struct json_object *cur = NULL;
    json_object_object_get_ex(depend_obj, name, &cur);
    for (int i = 0; i < json_object_array_length(cur); i++) {
        json_object *obj = json_object_array_get_idx(cur, i);
        const char *n = json_object_get_string(obj);
        if (strncmp(dep_name, n, strlen(dep_name)) == 0)
            return true;
    }
    json_object_put(cur);
    return false;
}

static bool
in_list(const char *name, list_head *list)
{
    list_head *p, *n;
    wb_item *item;

    list_for_each_safe(p, n, list) {
        item = list_entry(p, wb_item, list);
        if (strncmp(name, item->mod_name, strlen(name)) == 0)
            return true;
    }
    return false;
}

static void
traverse_dependency(module *mod,
                    json_object *json_top, bool verify,
                    int *num_profile_mods, uint64_t *profile_mods,
                    sort_callback cb, FILE *fp,
                    list_head *whitelist, list_head *blacklist)
{
    list_head *p, *n;

    //printf("### %s: '%s' (%d)\n", __func__, mod->name, mod->status);

    if (mod->status)
        return;

    mod->status = M_STATUS_DOING;

    json_object *dep_mods = NULL;

    int handled = 0;
    list_for_each_safe(p, n, &mod->dependencies) {
        depend *d = list_entry(p, depend, list);

        if (d->flags & DEPEND_FLAGS_CANDIDATE) {
            if (in_list(d->mod->name, blacklist)) {
                handled++;
                continue;
            }

            if (!in_list(d->mod->name, whitelist))
                need_handle_candidates = true;

            printf("%s: item '%s'\n", __func__, d->mod->name);
        }

        printf("###### %s: '%s' -> '%s'\n", __func__, mod->name, d->mod->name);

        traverse_dependency(d->mod, json_top, verify,
                            num_profile_mods, profile_mods,
                            cb, fp, whitelist, blacklist);
        if (d->mod->status != M_STATUS_DONE) {
            printf("%s: cyclic chain: '%s'\n", __func__, d->mod->name);
            return;
        }

        if (verify) {
#if 0
            printf("%s: verify '%s' depend '%s'\n",
                   __func__, mod->name, d->mod->name);
#endif
            if (!verify_dependency(mod->name, d->mod->name, json_top)) {
                printf("%s: no dependency '%s' -> '%s' in this profile.\n",
                       __func__, mod->name, d->mod->name);
                exit(-1);
            }
        } else {
            if (dep_mods == NULL)
                dep_mods = json_object_new_array();

            if (strncmp(d->mod->name, "startup", strlen("startup")) != 0) {
                json_object_array_add(dep_mods,
                                      json_object_new_string(d->mod->name));
            }
        }

        handled++;
        //printf("%s: end '%s' -> '%s'\n", __func__, mod->name, d->mod->name);
    }

    if (!verify) {
        struct json_object *depend_obj = NULL;
        json_object_object_get_ex(json_top, "dependencies", &depend_obj);
        assert(depend_obj != NULL);
        json_object_object_add(depend_obj, mod->name, dep_mods);
    }

    if (handled != mod->num_dependencies) {
        printf("'%s' broken dependencies!\n", mod->name);
        exit(-1);
    }

    mod->status = M_STATUS_DONE;
    cb(mod, num_profile_mods, profile_mods);
}

static void
write_profile_to_bootrd(int num_profile_mods, uint64_t *profile_mods,
                        struct bootrd_header *hdr, FILE *fp,
                        const char *name)
{
    /* Remove prefix and suffix of the name */
    if (strncmp(name, "top_", strlen("top_") != 0)) {
        printf("%s: bad name '%s' which must start with 'top_'\n",
               __func__, name);
        exit(-1);
    }

    struct profile_header header;
    memset(&header, 0, sizeof(header));
    memcpy(&header.magic, &PROFILE_MAGIC, sizeof(header.magic));
    header.version = 1;
    header.total_size = sizeof(struct profile_header) +
        sizeof(uint64_t) * num_profile_mods;
    header.mod_num = num_profile_mods;

    size_t len = 0;
    const char *end_name = strrchr(name, '.');
    if (end_name)
        len = end_name - name;
    else
        len = strlen(name);
    len -= strlen("top_");
    len = MIN(len, (sizeof(header.sname) - 1));
    strncpy(header.sname, name + strlen("top_"), len);

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        printf("%s: cannot write profile header to bootrd file!\n",
               __func__);
        exit(-1);
    }
#if 0
    printf("###### %s: num_profile_mods (%d)\n", __func__, num_profile_mods);
    {
        int i = 0;
        for (i = 0; i < num_profile_mods; i++) {
            printf("###### %s: (%ld)\n", __func__, profile_mods[i]);
        }
    }
#endif
    uint64_t ret;
    ret = fwrite(profile_mods, sizeof(uint64_t), num_profile_mods, fp);
    if (ret != num_profile_mods) {
        printf("%s: cannot write profile data into bootrd file!\n", __func__);
        exit(-1);
    }

    hdr->profile_num++;
}

static void
reset_modules_status(void)
{
    list_head *p, *n;
    module *mod;

    list_for_each_safe(p, n, &modules) {
        mod = list_entry(p, module, list);
        mod->status = M_STATUS_NONE;
    }
}

static void
append_fixups_to_profile(json_object *json_top)
{
    if (list_empty(&fixups))
        return;

    struct json_object *depend_obj = NULL;
    json_object_object_get_ex(json_top, "dependencies", &depend_obj);
    assert(depend_obj != NULL);

    json_object *json_fixups = json_object_new_array();
    json_object_object_add(json_top, "fixups", json_fixups);

    depend *fixup;
    list_for_each_entry(fixup, &fixups, list) {
        json_object *candidates = NULL;

        depend *p = fixup->next;
        while (p) {
            struct json_object *obj = NULL;
            json_object_object_get_ex(depend_obj, p->mod->name, &obj);
            if (obj == NULL)
                break;

            printf("############### Candidates: %s: mod '%s'\n",
                   __func__, p->mod->name);

            if (candidates == NULL)
                candidates = json_object_new_array();

            json_object_array_add(candidates,
                                  json_object_new_string(p->mod->name));

            p = p->next;
        }

        if (candidates == NULL)
            continue;

        json_object *json_fixup = json_object_new_object();
        json_object_object_add(json_fixup, "candidates", candidates);
        json_object_object_add(json_fixup, "select",
                               json_object_new_string(""));

        json_object_array_add(json_fixups, json_fixup);
    }
}

static void
setup_white_black_list(json_object *fixups,
                       list_head *whitelist,
                       list_head *blacklist)
{
    for (int i = 0; i < json_object_array_length(fixups); i++) {
        json_object *obj = json_object_array_get_idx(fixups, i);

        json_object *select = NULL;
        json_object_object_get_ex(obj, "select", &select);
        if (select == NULL)
            goto err;

        const char *str_select = json_object_get_string(select);
        if (str_select == NULL || strlen(str_select) == 0)
            goto err;
        printf("%s: select '%s'\n", __func__, str_select);
        wb_item *item = calloc(1, sizeof(wb_item));
        item->mod_name = strdup(str_select);
        list_add_tail(&(item->list), whitelist);

        struct json_object *candidates = NULL;
        json_object_object_get_ex(obj, "candidates", &candidates);
        if (candidates == NULL)
            goto err;

        for (int i = 0; i < json_object_array_length(candidates); i++) {
            json_object *candidate = json_object_array_get_idx(candidates, i);
            const char *n = json_object_get_string(candidate);
            printf("%s: candidate '%s'\n", __func__, n);
            if (strncmp(str_select, n, strlen(str_select)) != 0) {
                wb_item *item = calloc(1, sizeof(wb_item));
                item->mod_name = strdup(n);
                list_add_tail(&(item->list), blacklist);
            }
        }
    }

    return;

 err:
    printf("Fill in 'select' from 'candidates' for each fixup items.\n");
}

void
sort_modules(struct bootrd_header *hdr, sort_callback cb, FILE *fp)
{
    list_head *p, *n;
    module *mod;

    /* Init startup module */
    mod_startup.name = "startup";
    mod_startup.status = M_STATUS_DONE;

    discover_modules(fp);
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

    /* Traverse modules based on their dependency chains,
     * try to build bootrd and profiles */
    list_for_each_entry(mod, &modules, list) {
        char filename[260];
        /* Only top_xxx can act as start point */
        if (strncmp(mod->name, "top_", 4))
            continue;

        list_head whitelist;
        INIT_LIST_HEAD(&whitelist);
        list_head blacklist;
        INIT_LIST_HEAD(&blacklist);

        int num_profile_mods = 0;
        uint64_t *profile_mods = calloc(num_modules, sizeof(uint64_t));

        /* If profile exists, we just verify dependencies based on it;
         * Or we will create a profile and fill in all dependencies. */
        bool verify = true;
        sprintf(filename, "./%s.json", mod->name);
        json_object *json_top = json_object_from_file(filename);
        if (json_top != NULL) {
            json_object *json_fixups;
            json_object_object_get_ex(json_top, "fixups", &json_fixups);
            if (json_fixups != NULL) {
                printf("################################## fixups\n");
                setup_white_black_list(json_fixups, &whitelist, &blacklist);
            }
        } else {
            json_top = json_object_new_object();
            json_object *depend_obj = json_object_new_object();
            json_object_object_add(json_top, "dependencies", depend_obj);
            verify = false;
        }

        traverse_dependency(mod, json_top, verify,
                            &num_profile_mods, profile_mods,
                            cb, fp, &whitelist, &blacklist);
        if (mod->status != M_STATUS_DONE) {
            printf("%s: cyclic chain: '%s'.\n", __func__, mod->name);
            exit(-1);
        }

        if (!verify) {
            /* Create module profile(json-style) for human */
            append_fixups_to_profile(json_top);

            json_object_to_file(filename, json_top);
        }
        json_object_put(json_top);

        /* Add module profiles into bootrd */
        if (hdr->profile_offset == 0) {
            hdr->profile_offset = ftell(fp);
            hdr->current_profile = hdr->profile_offset;
        }
#if 0
        if (strncmp(mod->name, "top_linux",
                    strlen("top_linux")) == 0)
            hdr->current_profile = ftell(fp);
        else if (strncmp(mod->name, "top_hello_world",
                         strlen("top_hello_world")) == 0)
            hdr->current_profile = ftell(fp);
#else
        if (strncmp(mod->name, "top_arceos", strlen("top_arceos")) == 0)
            hdr->current_profile = ftell(fp);
#endif

        write_profile_to_bootrd(num_profile_mods, profile_mods,
                                hdr, fp, mod->name);

        free(profile_mods);
        profile_mods = NULL;
        num_profile_mods = 0;

        reset_modules_status();
    }
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
sort_func(module *mod, int *num_profile_mods, uint64_t *profile_mods)
{
    if (*num_profile_mods == num_modules) {
        printf("%s: find too many modules %d, limit is %d!\n",
               __func__, *num_profile_mods, num_modules);
        exit(-1);
    }
    profile_mods[(*num_profile_mods)++] = mod->offset_in_file;
#if 0
    printf("%s: mod '%s'; offset '%ld'\n",
           __func__, mod->name, mod->offset_in_file);
#endif
}

FILE *
create_bootrd(struct bootrd_header *hdr)
{
    memcpy(&(hdr->magic), &BOOTRD_MAGIC, sizeof(hdr->magic));
    hdr->version = 1;
    hdr->mod_offset = sizeof(*hdr);

    char filename[256] = {0};
    sprintf(filename, "%s/%s", kmod_dir, BOOTRD_FILENAME);
    FILE *fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("%s: no base bootrd file [%s]!\n", __func__, filename);
        exit(-1);
    }
    fseek(fp, sizeof(*hdr), SEEK_SET);
    return fp;
}

void
complete_bootrd(struct bootrd_header *hdr, FILE *fp)
{
    if (need_handle_candidates) {
        fclose(fp);
        clean_bootrd();

        printf("Find conflicts, reserve only one among candidates!\n");
        printf("Then run mk_bootfd again!\n");
        return;
    }

    hdr->total_size = ftell(fp);

#if 1
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
    if (ftell(fp) > BOOTRD_SIZE) {
        printf("%s: bad bootrd file (%ld > 32M).\n",
               __func__, ftell(fp));
        exit(-1);
    }

    /* overwrite bootrd header */
    fseek(fp, 0, SEEK_SET);
    if (fwrite(hdr, sizeof(*hdr), 1, fp) != 1) {
        printf("%s: cannot overwrite bootrd header!\n", __func__);
        exit(-1);
    }

    /* extend bootrd disk to specific size */
    fseek(fp, BOOTRD_SIZE - 1, SEEK_SET);
    char zero = 0;
    if (fwrite(&zero, sizeof(zero), 1, fp) != 1) {
        printf("%s: cannot extend bootrd disk!\n", __func__);
        exit(-1);
    }

    fclose(fp);
}

int
main(int argc, char *argv[])
{
    kmod_dir = getenv("KMODULE_DIR");
    if (kmod_dir == NULL) {
        if (argc != 2) {
            printf("no module output directory, "
                   "set env 'KMODULE_DIR=[path]' or input the path\n");
            exit(-1);
        }
        kmod_dir = argv[1];
    }
    printf("KMODULE_DIR: %s\n", kmod_dir);

    struct bootrd_header hdr;
    memset(&hdr, 0, sizeof(hdr));

    FILE* fp = create_bootrd(&hdr);
    sort_modules(&hdr, sort_func, fp);
    complete_bootrd(&hdr, fp);
    clear_symbols();
    clear_modules();

    return 0;
}
