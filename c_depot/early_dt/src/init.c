// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <fdt.h>
#include <page.h>
#include <params.h>
#include <string.h>
#include <export.h>
#include <memblock.h>

#define MIN_MEMBLOCK_ADDR   __pa(PAGE_OFFSET)
#define MAX_MEMBLOCK_ADDR   ((phys_addr_t)~0)

extern void *dtb_early_va;

void *initial_boot_params;
EXPORT_SYMBOL(initial_boot_params);

/* Untouched command line saved by arch-specific code. */
char boot_command_line[COMMAND_LINE_SIZE];
EXPORT_SYMBOL(boot_command_line);

int dt_root_addr_cells;
int dt_root_size_cells;

phys_addr_t dt_memory_base = 0;
EXPORT_SYMBOL(dt_memory_base);
phys_addr_t dt_memory_size = 0;
EXPORT_SYMBOL(dt_memory_size);

static u64
dt_mem_next_cell(int s, const u32 **cellp)
{
    const u32 *p = *cellp;

    *cellp = p + s;
    return of_read_number(p, s);
}

static void
early_init_dt_add_memory_arch(u64 base, u64 size)
{
    const u64 phys_offset = MIN_MEMBLOCK_ADDR;

    if (size < PAGE_SIZE - (base & ~PAGE_MASK)) {
        printk("Ignoring memory block %lx - %lx\n",
               base, base + size);
        return;
    }

    if (!PAGE_ALIGNED(base)) {
        size -= PAGE_SIZE - (base & ~PAGE_MASK);
        base = PAGE_ALIGN(base);
    }
    size &= PAGE_MASK;

    if (base > MAX_MEMBLOCK_ADDR) {
        printk("Ignoring memory block %lx - %lx\n",
               base, base + size);
        return;
    }

    if (base + size - 1 > MAX_MEMBLOCK_ADDR) {
        printk("Ignoring memory range %lx - %lx\n",
               ((u64)MAX_MEMBLOCK_ADDR) + 1, base + size);
        size = MAX_MEMBLOCK_ADDR - base + 1;
    }

    if (base + size < phys_offset) {
        printk("Ignoring memory block %lx - %lx\n",
               base, base + size);
        return;
    }

    if (base < phys_offset) {
        printk("Ignoring memory range %lx - %lx\n",
               base, phys_offset);
        size -= phys_offset - base;
        base = phys_offset;
    }

    dt_memory_base = base;
    dt_memory_size = size;
}

int
fdt_check_header(const void *fdt)
{
    if (fdt_magic(fdt) != FDT_MAGIC)
        return -FDT_ERR_BADMAGIC;

    return 0;
}
EXPORT_SYMBOL(fdt_check_header);

const char *
fdt_get_name(const void *fdt, int nodeoffset, int *len)
{
    const struct fdt_node_header *nh = fdt_offset_ptr_(fdt, nodeoffset);
    const char *nameptr = nh->name;

    if (len)
        *len = strlen(nameptr);

    return nameptr;
}
EXPORT_SYMBOL(fdt_get_name);

bool
early_init_dt_verify(void)
{
    if (dtb_early_va == NULL)
        panic("dtb_early_va is NULL!");

    if (fdt_check_header(dtb_early_va))
        return false;

    /* Setup flat device-tree pointer */
    initial_boot_params = dtb_early_va;
    return true;
}
EXPORT_SYMBOL(early_init_dt_verify);

static uint32_t
fdt_next_tag(const void *fdt, int startoffset, int *nextoffset)
{
    const fdt32_t *tagp;
    const fdt32_t *lenp;
    uint32_t tag;
    const char *p;
    int offset = startoffset;

    tagp = fdt_offset_ptr_(fdt, offset);
    if (!tagp)
        return FDT_END; /* premature end */

    tag = fdt32_to_cpu(*tagp);
    offset += FDT_TAGSIZE;

    switch (tag) {
    case FDT_BEGIN_NODE:
        /* skip name */
        do {
            p = fdt_offset_ptr_(fdt, offset++);
        } while (p && (*p != '\0'));
        if (!p)
            return FDT_END; /* premature end */
        break;

    case FDT_PROP:
        lenp = fdt_offset_ptr_(fdt, offset);
        if (!lenp)
            return FDT_END; /* premature end */
        /* skip-name offset, length and value */
        offset += sizeof(struct fdt_property)
            - FDT_TAGSIZE + fdt32_to_cpu(*lenp);
        break;

    case FDT_END:
    case FDT_END_NODE:
    case FDT_NOP:
        break;

    default:
        return FDT_END;
    }

    if (!fdt_offset_ptr_(fdt, startoffset))
        return FDT_END; /* premature end */

    *nextoffset = FDT_TAGALIGN(offset);
    return tag;
}

int
fdt_next_node(const void *fdt, int offset, int *depth)
{
    uint32_t tag;
    int nextoffset = 0;

    if (offset >= 0)
        fdt_next_tag(fdt, offset, &nextoffset);

    do {
        offset = nextoffset;
        tag = fdt_next_tag(fdt, offset, &nextoffset);

        switch (tag) {
        case FDT_PROP:
        case FDT_NOP:
            break;

        case FDT_BEGIN_NODE:
            if (depth)
                (*depth)++;
            break;

        case FDT_END_NODE:
            if (depth && ((--(*depth)) < 0))
                return nextoffset;
            break;

        case FDT_END:
            if ((nextoffset >= 0)
                || ((nextoffset == -FDT_ERR_TRUNCATED) && !depth))
                return -FDT_ERR_NOTFOUND;
            else
                return nextoffset;
        }
    } while (tag != FDT_BEGIN_NODE);

    return offset;
}
EXPORT_SYMBOL(fdt_next_node);

/**
 * of_scan_flat_dt - scan flattened tree blob and call callback on each.
 * @it: callback function
 * @data: context data pointer
 *
 * This function is used to scan the flattened device-tree, it is
 * used to extract the memory information at boot before we can
 * unflatten the tree
 */
int
of_scan_flat_dt(of_scan_flat_dt_cb cb, void *data)
{
    const void *blob = initial_boot_params;
    const char *pathp;
    int offset, rc = 0, depth = -1;

    if (!blob)
        return 0;

    for (offset = fdt_next_node(blob, -1, &depth);
         offset >= 0 && depth >= 0 && !rc;
         offset = fdt_next_node(blob, offset, &depth)) {

        pathp = fdt_get_name(blob, offset, NULL);
        rc = cb(offset, pathp, depth, data);
    }

    return rc;
}

static int
nextprop_(const void *fdt, int offset)
{
    uint32_t tag;
    int nextoffset;

    do {
        tag = fdt_next_tag(fdt, offset, &nextoffset);

        switch (tag) {
        case FDT_END:
            if (nextoffset >= 0)
                return -FDT_ERR_BADSTRUCTURE;
            else
                return nextoffset;

        case FDT_PROP:
            return offset;
        }
        offset = nextoffset;
    } while (tag == FDT_NOP);

    return -FDT_ERR_NOTFOUND;
}

int
fdt_first_property_offset(const void *fdt, int nodeoffset)
{
    int offset;
    fdt_next_tag(fdt, nodeoffset, &offset);
    return nextprop_(fdt, offset);
}
EXPORT_SYMBOL(fdt_first_property_offset);

int
fdt_next_property_offset(const void *fdt, int offset)
{
    fdt_next_tag(fdt, offset, &offset);
    return nextprop_(fdt, offset);
}
EXPORT_SYMBOL(fdt_next_property_offset);

const struct fdt_property *
fdt_get_property_by_offset_(const void *fdt, int offset, int *lenp)
{
    const struct fdt_property *prop;

    prop = fdt_offset_ptr_(fdt, offset);

    if (lenp)
        *lenp = fdt32_ld(&prop->len);

    return prop;
}
EXPORT_SYMBOL(fdt_get_property_by_offset_);

const char *
fdt_get_string(const void *fdt, int stroffset, int *lenp)
{
    uint32_t absoffset;
    size_t len;
    const char *s, *n;

    absoffset = stroffset + fdt_off_dt_strings(fdt);
    len = fdt_size_dt_strings(fdt) - stroffset;
    s = (const char *)fdt + absoffset;
    n = memchr(s, '\0', len);

    if (lenp)
        *lenp = n - s;
    return s;
}
EXPORT_SYMBOL(fdt_get_string);

static int
fdt_string_eq_(const void *fdt, int stroffset, const char *s, int len)
{
    int slen;
    const char *p = fdt_get_string(fdt, stroffset, &slen);

    return p && (slen == len) && (memcmp(p, s, len) == 0);
}

static const struct fdt_property *
fdt_get_property_namelen_(const void *fdt,
                          int offset,
                          const char *name,
                          int namelen,
                          int *lenp,
                          int *poffset)
{
    for (offset = fdt_first_property_offset(fdt, offset);
         (offset >= 0);
         (offset = fdt_next_property_offset(fdt, offset))) {
        const struct fdt_property *prop;

        prop = fdt_get_property_by_offset_(fdt, offset, lenp);
        if (!prop) {
            offset = -FDT_ERR_INTERNAL;
            break;
        }

        if (fdt_string_eq_(fdt, fdt32_ld(&prop->nameoff), name, namelen)) {
            if (poffset)
                *poffset = offset;
            return prop;
        }
    }

    if (lenp)
        *lenp = offset;
    return NULL;
}

static const void *
fdt_getprop_namelen(const void *fdt,
                    int nodeoffset,
                    const char *name,
                    int namelen,
                    int *lenp)
{
    int poffset;
    const struct fdt_property *prop;

    prop = fdt_get_property_namelen_(fdt, nodeoffset, name, namelen, lenp,
                     &poffset);
    if (!prop)
        return NULL;

    return prop->data;
}

static const void *
fdt_getprop(const void *fdt, int nodeoffset, const char *name, int *lenp)
{
    return fdt_getprop_namelen(fdt, nodeoffset, name, strlen(name), lenp);
}

/**
 * of_get_flat_dt_prop - Given a node in the flat blob, return the property ptr
 *
 * This function can be used within scan_flattened_dt callback to get
 * access to properties
 */
const void *
of_get_flat_dt_prop(unsigned long node, const char *name, int *size)
{
    return fdt_getprop(initial_boot_params, node, name, size);
}

static int
early_init_dt_scan_root(unsigned long node,
                        const char *uname,
                        int depth,
                        void *data)
{
    const u32 *prop;

    if (depth != 0)
        return 0;

    dt_root_size_cells = OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
    dt_root_addr_cells = OF_ROOT_NODE_ADDR_CELLS_DEFAULT;

    prop = of_get_flat_dt_prop(node, "#size-cells", NULL);
    if (prop)
        dt_root_size_cells = be32_to_cpup(prop);

    prop = of_get_flat_dt_prop(node, "#address-cells", NULL);
    if (prop)
        dt_root_addr_cells = be32_to_cpup(prop);

    return 1;
}

static int
early_init_dt_scan_memory(unsigned long node,
                          const char *uname,
                          int depth,
                          void *data)
{
    const u32 *reg;
    const u32 *endp;
    const char *type;
    int len;

    type = of_get_flat_dt_prop(node, "device_type", NULL);
    if (type == NULL || strcmp(type, "memory") != 0)
        return 0;

    reg = of_get_flat_dt_prop(node, "reg", &len);
    if (reg == NULL)
        return 0;

    endp = reg + (len / sizeof(u32));

    printk("memory scan node %s, reg size %d,\n", uname, len);

    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells)) {
        u64 base, size;

        base = dt_mem_next_cell(dt_root_addr_cells, &reg);
        size = dt_mem_next_cell(dt_root_size_cells, &reg);

        if (size == 0)
            continue;

        printk(" - %lx ,  %lx\n",
               (unsigned long)base,
               (unsigned long)size);

        early_init_dt_add_memory_arch(base, size);
    }

    return 0;
}

static int
early_init_dt_scan_chosen(unsigned long node, const char *uname,
                          int depth, void *data)
{
    int l;
    const char *p;

    if (depth != 1 || !data ||
        (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
        return 0;

    /* Retrieve command line */
    p = of_get_flat_dt_prop(node, "bootargs", &l);
    if (p != NULL && l > 0)
        strlcpy(data, p, min(l, COMMAND_LINE_SIZE));

    printk("Command line is: %s\n", (char *)data);

    /* break now */
    return 1;
}

void
early_init_dt_scan_nodes(void)
{
    if (!of_scan_flat_dt(early_init_dt_scan_chosen, boot_command_line))
        panic("No chosen node found!");

    of_scan_flat_dt(early_init_dt_scan_root, NULL);

    of_scan_flat_dt(early_init_dt_scan_memory, NULL);
}
EXPORT_SYMBOL(early_init_dt_scan_nodes);

char saved_root_name[64];
EXPORT_SYMBOL(saved_root_name);

static int
root_dev_setup(char *param, char *value)
{
    strlcpy(saved_root_name, value, sizeof(saved_root_name));
    return 0;
}

static struct kernel_param kernel_params[] = {
    { .name = "root", .setup_func = root_dev_setup, },
    { .name = "console", .setup_func = console_setup, },
};

static unsigned int
num_kernel_params = sizeof(kernel_params) / sizeof(struct kernel_param);

int
init_module(void)
{
    printk("module[early_dt]: init begin ...\n");
    early_init_dt_verify();
    early_init_dt_scan_nodes();

    BUG_ON(parse_args(boot_command_line, kernel_params, num_kernel_params));
    printk("Memory Region (based on fdt): %lx - %lx\n",
           dt_memory_base, dt_memory_base + dt_memory_size);
    printk("module[early_dt]: init end!\n");
    return 0;
}
