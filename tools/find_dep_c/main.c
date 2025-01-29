// SPDX-License-Identifier: GPL-2.0-only

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <json.h>
#include <assert.h>
#include <unistd.h>

#include "module.h"
#include "../../booter/src/bootrd.h"

#define MIN(i, j) (((i) < (j)) ? (i) : (j))

/* n must be power of 2 */
#define ROUND_UP(x, n) (((x) + (n) - 1UL) & ~((n) - 1UL))

#define BOOTRD_FILENAME "bootrd.disk"

/* Bootrd file is used as qemu.pflash */
#define BOOTRD_SIZE 32*1024*1024L   /* 32M */

/* The main output directory for modules */
char *kmod_dir = NULL;
char *top = NULL;

typedef depend* (*match_callback)(module *mod, symbol *sym);
static bool need_handle_candidates = false;

static uint32_t ksym_ptr;
static uint32_t ksym_num;

static int num_modules = 0;
static LIST_HEAD(modules);
static LIST_HEAD(symbols);
static LIST_HEAD(fixups);

static uint8_t *
read_module(const char *filename, long *psize);

/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
char *skip_spaces(const char *str)
{
    while (isspace(*str))
        ++str;
    return (char *)str;
}

/**
 * strim - Removes leading and trailing whitespace from @s.
 * @s: The string to be stripped.
 *
 * Note that the first trailing whitespace is replaced with a %NUL-terminator
 * in the given string @s. Returns a pointer to the first non-whitespace
 * character in @s.
 */
char *strim(char *s)
{
    size_t size;
    char *end;

    size = strlen(s);
    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    return skip_spaces(s);
}

static void
terminate()
{
    exit(-1);
}

static bool
check_module(const char *name)
{
    char *p;

    p = strrchr(name, '.');
    return !strcmp(p, ".ko");
}

static void
discover_modules()
{
    int n;
    struct dirent **namelist;
    bool has_top = false;

    n = scandir(kmod_dir, &namelist, NULL, NULL);
    if (n == -1) {
        printf("%s: error when scandir!\n", __func__);
        exit(-1);
    }

    while (n--) {
        if (check_module(namelist[n]->d_name)) {
            module *mod = calloc(1, sizeof(module));
            mod->name = strdup(namelist[n]->d_name);
            //printf("%s: %s\n", mod->name, top);
            INIT_LIST_HEAD(&(mod->undef_syms));
            INIT_LIST_HEAD(&(mod->dependencies));
            mod->num_dependencies = 0;
            list_add_tail(&(mod->list), &modules);
            num_modules++;

            if (!strncmp(namelist[n]->d_name, top, strlen(top))) {
                has_top = true;
            }
        }
        free(namelist[n]);
    }
    free(namelist);

    if (!has_top) {
        printf("%s: No top '%s'\n", __func__, top);
        exit(-1);
    }
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

static char *
get_strtab(Elf64_Shdr *shdr, FILE *fp)
{
    char *str;

    str = (char *) calloc(shdr->sh_size, 1);

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
    //printf("%s: mod '%s' sym '%s'\n", __func__, mod->name, sym->name);

    depend *d = find_dependency(mod, sym->mod);
    if (d == NULL) {
        d = calloc(1, sizeof(depend));
        d->mod = sym->mod;
        list_add_tail(&(d->list), &(mod->dependencies));
        mod->num_dependencies++;
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
        //terminate();
        return;
    }

    if (head->next == NULL)
        return;

    /* Okay, now there're more than one module that can solve
     * this undef symbol. Report this in the profile and require
     * manual intervention. */
    depend *p = head;
    while (p) {
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

    printf("%s: mod '%s'\n", __func__, mod->name);
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

            /* Discover all undef syms for this mod */
            discover_undef_syms(mod, shdr, strtab, fp);
            free(strtab);
        } else {
            const char *name = secstrings + shdr->sh_name;
            if (strcmp(name, "__ksymtab_strings") == 0) {
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

static bool
verify_dependency(const char *name, const char *dep_name,
                  json_object *json_top)
{
    printf("%s: verify '%s' depend '%s'\n", __func__, name, dep_name);

    /* Of course, module 'lds' always exists. */
    if (strncmp(dep_name, "lds", strlen("lds")) == 0)
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

static void
traverse_dependency(module *mod,
                    json_object *json_top, bool verify,
                    int *num_profile_mods, uint64_t *profile_mods,
                    sort_callback cb)
{
    list_head *p, *n;

    printf("### BEGIN %s: '%s' (%d)\n", __func__, mod->name, mod->status);

    if (mod->status)
        return;

    mod->status = M_STATUS_DOING;

    json_object *dep_mods = NULL;

    int handled = 0;
    list_for_each_safe(p, n, &mod->dependencies) {
        depend *d = list_entry(p, depend, list);

        traverse_dependency(d->mod, json_top, verify,
            num_profile_mods, profile_mods, cb);
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

            json_object_array_add(dep_mods,
                json_object_new_string(d->mod->name));
        }

        handled++;
        printf("%s: end '%s' -> '%s'\n", __func__, mod->name, d->mod->name);
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
    printf("### END %s: '%s' (%d)\n", __func__, mod->name, mod->status);
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

static void
add_lds_symbols()
{
    int ret;
    char cwd[PATH_MAX];
    if (readlink("/proc/self/exe", cwd, PATH_MAX) < 0) {
        printf("Cannot find current path.\n");
        exit(-1);
    }
    char *pos = strrchr(cwd, '/');
    if (pos == NULL) {
        printf("Bad current path: %s.", cwd);
        exit(-1);
    }
    *pos = '\0';
    strcat(cwd, "/lds.conf");
    //printf("cwd: %s\n", cwd);

    FILE *fp = fopen(cwd, "r");
    if (fp == NULL) {
        printf("No lds.conf at current path.\n");
        exit(-1);
    }

    module *mod = calloc(1, sizeof(module));
    mod->name = strdup("lds");
    INIT_LIST_HEAD(&(mod->undef_syms));
    INIT_LIST_HEAD(&(mod->dependencies));
    mod->num_dependencies = 0;
    num_modules++;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        strim(line);
        if (strlen(line) == 0)
            break;

        symbol *sym = calloc(1, sizeof(symbol));
        sym->name = strdup(line);
        sym->mod = mod;
        list_add_tail(&(sym->list), &symbols);
    }
    fclose(fp);
}

void
sort_modules(sort_callback cb)
{
    list_head *p, *n;
    module *mod;

    discover_modules();

    /* For each module, register its exported symbols into a list */
    list_for_each_entry(mod, &modules, list)
        analysis_module(mod);

    /* Add symbols of lds into symbols list */
    add_lds_symbols();

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
        if (strncmp(mod->name, top, strlen(top)))
            continue;

        int num_profile_mods = 0;
        uint64_t *profile_mods = calloc(num_modules, sizeof(uint64_t));

        /* If profile exists, we just verify dependencies based on it;
         * Or we will create a profile and fill in all dependencies. */
        bool verify = true;
        sprintf(filename, "./%s/%s.json", kmod_dir, mod->name);
        json_object *json_top = json_object_from_file(filename);
        if (json_top == NULL) {
            json_top = json_object_new_object();
            json_object *depend_obj = json_object_new_object();
            json_object_object_add(json_top, "dependencies", depend_obj);
            verify = false;
        }

    printf("=================> (%s)\n", mod->name);
        traverse_dependency(mod, json_top, verify,
            &num_profile_mods, profile_mods, cb);

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
#if 1
    printf("===> %s: mod '%s'; offset '%ld'\n",
           __func__, mod->name, mod->offset_in_file);
#endif
}

int
main(int argc, char *argv[])
{
    top = getenv("TOP_COMPONENT");
    if (top == NULL) {
        printf("no top component! set env 'TOP_COMPONENT=[top_name]'\n");
        exit(-1);
    }

    kmod_dir = getenv("KMODULE_DIR");
    if (kmod_dir == NULL) {
        printf("no module output directory, set env 'KMODULE_DIR=[path]'\n");
        exit(-1);
    }

    sort_modules(sort_func);
    clear_symbols();
    clear_modules();

    printf("[OK]: Make BOOTRD at '%s'.\n", kmod_dir);
    return 0;
}
