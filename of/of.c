// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <fdt.h>
#include <slab.h>
#include <errno.h>
#include <driver.h>
#include <export.h>
#include <string.h>
#include <mod_devicetable.h>

struct device_node *of_aliases;
struct device_node *of_chosen;
struct device_node *of_stdout;
static const char *of_stdout_options;

struct device_node *of_root;
EXPORT_SYMBOL(of_root);

struct kobj_type of_node_ktype = {
};

static struct fwnode_handle *
of_fwnode_get(struct fwnode_handle *fwnode)
{
    return of_fwnode_handle(of_node_get(to_of_node(fwnode)));
}

static void
of_fwnode_put(struct fwnode_handle *fwnode)
{
    of_node_put(to_of_node(fwnode));
}

const struct fwnode_operations of_fwnode_ops = {
    .get = of_fwnode_get,
    .put = of_fwnode_put,
};
EXPORT_SYMBOL(of_fwnode_ops);

static struct device_node *
__of_get_next_child(const struct device_node *node, struct device_node *prev)
{
    struct device_node *next;

    if (!node)
        return NULL;

    next = prev ? prev->sibling : node->child;
    for (; next; next = next->sibling)
        if (of_node_get(next))
            break;
    of_node_put(prev);
    return next;
}
#define __for_each_child_of_node(parent, child) \
    for (child = __of_get_next_child(parent, NULL); child != NULL; \
         child = __of_get_next_child(parent, child))

struct device_node *
__of_find_node_by_path(struct device_node *parent, const char *path)
{
    struct device_node *child;
    int len;

    len = strcspn(path, "/:");
    if (!len)
        return NULL;

    __for_each_child_of_node(parent, child) {
        const char *name = kbasename(child->full_name);
        if (strncmp(path, name, len) == 0 && (strlen(name) == len))
            return child;
    }
    return NULL;
}

struct device_node *
__of_find_node_by_full_path(struct device_node *node, const char *path)
{
    const char *separator = strchr(path, ':');

    while (node && *path == '/') {
        struct device_node *tmp = node;

        path++; /* Increment past '/' delimiter */
        node = __of_find_node_by_path(node, path);
        of_node_put(tmp);
        path = strchrnul(path, '/');
        if (separator && separator < path)
            break;
    }
    return node;
}

struct device_node *
of_find_node_opts_by_path(const char *path, const char **opts)
{
    struct device_node *np = NULL;
    struct property *pp;
    unsigned long flags;
    const char *separator = strchr(path, ':');

    if (opts)
        *opts = separator ? separator + 1 : NULL;

    if (strcmp(path, "/") == 0)
        return of_node_get(of_root);

    /* The path could begin with an alias */
    if (*path != '/') {
        int len;
        const char *p = separator;

        if (!p)
            p = strchrnul(path, '/');
        len = p - path;

        /* of_aliases must not be NULL */
        if (!of_aliases)
            return NULL;

        for_each_property_of_node(of_aliases, pp) {
            if (strlen(pp->name) == len && !strncmp(pp->name, path, len)) {
                np = of_find_node_by_path(pp->value);
                break;
            }
        }
        if (!np)
            return NULL;
        path = p;
    }

    /* Step down the tree matching path components */
    if (!np)
        np = of_node_get(of_root);
    np = __of_find_node_by_full_path(np, path);
    return np;
}
EXPORT_SYMBOL(of_find_node_opts_by_path);

static void
of_alias_scan(void)
{
	of_aliases = of_find_node_by_path("/aliases");
	if (of_aliases)
        panic("no aliases!");

	of_chosen = of_find_node_by_path("/chosen");
	if (of_chosen == NULL)
		of_chosen = of_find_node_by_path("/chosen@0");

	if (of_chosen) {
        const char *name = NULL;
        if (of_property_read_string(of_chosen, "stdout-path", &name))
            of_property_read_string(of_chosen, "linux,stdout-path", &name);
		if (name)
			of_stdout = of_find_node_opts_by_path(name, &of_stdout_options);
    }
}

const char *
of_prop_next_string(struct property *prop, const char *cur)
{
    const void *curv = cur;

    if (!prop)
        return NULL;

    if (!cur)
        return prop->value;

    curv += strlen(cur) + 1;
    if (curv >= prop->value + prop->length)
        return NULL;

    return curv;
}
EXPORT_SYMBOL(of_prop_next_string);

int
of_property_read_string(const struct device_node *np,
                        const char *propname,
                        const char **out_string)
{
    const struct property *prop = of_find_property(np, propname, NULL);
    if (!prop)
        return -EINVAL;
    if (!prop->value)
        return -ENODATA;
    if (strnlen(prop->value, prop->length) >= prop->length)
        return -EILSEQ;
    *out_string = prop->value;
    return 0;
}
EXPORT_SYMBOL(of_property_read_string);

static void
reverse_nodes(struct device_node *parent)
{
	struct device_node *child, *next;

	/* In-depth first */
	child = parent->child;
	while (child) {
		reverse_nodes(child);

		child = child->sibling;
	}

	/* Reverse the nodes in the child list */
	child = parent->child;
	parent->child = NULL;
	while (child) {
		next = child->sibling;

		child->sibling = parent->child;
		parent->child = child;
		child = next;
	}
}

static void *
unflatten_dt_alloc(void **mem, unsigned long size, unsigned long align)
{
	void *res;

	*mem = PTR_ALIGN(*mem, align);
	res = *mem;
	*mem += size;

	return res;
}

static const void *
fdt_getprop_by_offset(const void *fdt, int offset,
                      const char **namep, int *lenp)
{
    const struct fdt_property *prop;

    prop = fdt_get_property_by_offset_(fdt, offset, lenp);
    if (!prop)
        return NULL;

    if (namep) {
        const char *name;
        int namelen;

        name = fdt_get_string(fdt, fdt32_ld(&prop->nameoff), &namelen);
        if (!name) {
            if (lenp)
                *lenp = namelen;
            return NULL;
        }

        *namep = name;
    }

    return prop->data;
}

static void
populate_properties(const void *blob,
                    int offset,
                    void **mem,
                    struct device_node *np,
                    const char *nodename,
                    bool dryrun)
{
	int cur;
    struct property *pp;
	struct property **pprev = NULL;

	pprev = &np->properties;
	for (cur = fdt_first_property_offset(blob, offset);
	     cur >= 0;
	     cur = fdt_next_property_offset(blob, cur)) {
		const u32 *val;
		const char *pname;
		u32 sz;

		val = fdt_getprop_by_offset(blob, cur, &pname, &sz);
		if (!val) {
			panic("Cannot locate property at %x\n", cur);
			continue;
		}

		if (!pname) {
			panic("Cannot find property name at %x\n", cur);
			continue;
		}

		pp = unflatten_dt_alloc(mem, sizeof(struct property),
                                __alignof__(struct property));
		if (dryrun)
			continue;

		if (!strcmp(pname, "phandle") ||
		    !strcmp(pname, "linux,phandle")) {
			if (!np->phandle)
				np->phandle = be32_to_cpup(val);
		}

		pp->name   = (char *)pname;
		pp->length = sz;
		pp->value  = (u32 *)val;
		*pprev     = pp;
		pprev      = &pp->next;
    }

	{
		const char *p = nodename, *ps = p, *pa = NULL;
		int len;

		while (*p) {
			if ((*p) == '@')
				pa = p;
			else if ((*p) == '/')
				ps = p + 1;
			p++;
		}

		if (pa < ps)
			pa = p;
		len = (pa - ps) + 1;
		pp = unflatten_dt_alloc(mem, sizeof(struct property) + len,
                                __alignof__(struct property));
		if (!dryrun) {
			pp->name   = "name";
			pp->length = len;
			pp->value  = pp + 1;
			*pprev     = pp;
			pprev      = &pp->next;
			memcpy(pp->value, ps, len - 1);
			((char *)pp->value)[len - 1] = 0;
		}
	}

	if (!dryrun)
		*pprev = NULL;
}

static bool
populate_node(const void *blob,
              int offset,
              void **mem,
              struct device_node *dad,
              struct device_node **pnp,
              bool dryrun)
{
	struct device_node *np;
	const char *pathp;
    unsigned int l;
    unsigned int allocl;

	pathp = fdt_get_name(blob, offset, &l);
	if (!pathp) {
		*pnp = NULL;
		return false;
	}

	allocl = ++l;

	np = unflatten_dt_alloc(mem, sizeof(struct device_node) + allocl,
                            __alignof__(struct device_node));
	if (!dryrun) {
		char *fn;
		of_node_init(np);
		np->full_name = fn = ((char *)np) + sizeof(*np);

		memcpy(fn, pathp, l);

		if (dad != NULL) {
			np->parent = dad;
			np->sibling = dad->child;
			dad->child = np;
		}
	}

	populate_properties(blob, offset, mem, np, pathp, dryrun);
	if (!dryrun) {
		np->name = of_get_property(np, "name", NULL);
		if (!np->name)
			np->name = "<NULL>";
	}

	*pnp = np;
    return true;
}

const void *
__of_get_property(const struct device_node *np,
                  const char *name, int *lenp)
{
    struct property *pp = __of_find_property(np, name, lenp);

    return pp ? pp->value : NULL;
}
EXPORT_SYMBOL(__of_get_property);

const void *
of_get_property(const struct device_node *np, const char *name, int *lenp)
{
    struct property *pp = of_find_property(np, name, lenp);

    return pp ? pp->value : NULL;
}
EXPORT_SYMBOL(of_get_property);

struct device_node *
of_get_next_child(const struct device_node *node,
                  struct device_node *prev)
{
    struct device_node *next;
    next = __of_get_next_child(node, prev);
    return next;
}
EXPORT_SYMBOL(of_get_next_child);

static int
unflatten_dt_nodes(const void *blob,
                   void *mem,
                   struct device_node *dad,
                   struct device_node **nodepp)
{
	struct device_node *root;
	int offset = 0, depth = 0, initial_depth = 0;
#define FDT_MAX_DEPTH	64
	struct device_node *nps[FDT_MAX_DEPTH];
	void *base = mem;
	bool dryrun = !base;

	if (nodepp)
		*nodepp = NULL;

	if (dad)
		depth = initial_depth = 1;

	root = dad;
	nps[depth] = dad;

	for (offset = 0;
	     offset >= 0 && depth >= initial_depth;
	     offset = fdt_next_node(blob, offset, &depth)) {
		if (!populate_node(blob, offset, &mem, nps[depth],
                           &nps[depth+1], dryrun))
			return mem - base;

		if (!dryrun && nodepp && !*nodepp)
            *nodepp = nps[depth+1];
		if (!dryrun && !root)
			root = nps[depth+1];
    }

	if (offset < 0 && offset != -FDT_ERR_NOTFOUND) {
		panic("Error %d processing FDT\n", offset);
		return -EINVAL;
	}

	/*
	 * Reverse the child list. Some drivers assumes node order
     * matches .dts node order
	 */
	if (!dryrun)
		reverse_nodes(root);

	return mem - base;
}

void *
__unflatten_device_tree(const void *blob,
                        struct device_node *dad,
                        struct device_node **mynodes)
{
	int size;
	void *mem;

    printk(" -> unflatten_device_tree() blob(%lx)\n", blob);

    if (!blob) {
        printk("No device tree pointer\n");
        return NULL;
    }

    if (fdt_check_header(blob)) {
        printk("Invalid device tree blob header\n");
        return NULL;
    }

    printk("Unflattening device tree:\n");
    printk("magic: %x\n", fdt_magic(blob));
    printk("size: %x\n", fdt_totalsize(blob));
    printk("version: %x\n", fdt_version(blob));

	/* First pass, scan for size */
	size = unflatten_dt_nodes(blob, NULL, dad, NULL);
	if (size < 0)
		return NULL;

	size = _ALIGN(size, 4);

	/* Allocate memory for the expanded device tree */
    mem = (void *)__get_free_pages(GFP_KERNEL, order_base_2(PFN_UP(size)));
	if (!mem)
		return NULL;

	memset(mem, 0, size);

	*(u32 *)(mem + size) = cpu_to_be32(0xdeadbeef);

	printk("  unflattening %lx...\n", mem);

	/* Second pass, do actual unflattening */
	unflatten_dt_nodes(blob, mem, dad, mynodes);
	if (be32_to_cpup(mem + size) != 0xdeadbeef)
		panic("End of tree marker overwritten: %x\n",
              be32_to_cpup(mem + size));

	printk(" <- unflatten_device_tree()\n");
    return mem;
}

void
unflatten_device_tree(void)
{
    __unflatten_device_tree(initial_boot_params, NULL, &of_root);

    /* Get pointer to "/chosen" nodes for use everywhere */
    of_alias_scan();
    printk("stdout (%s)\n", of_stdout->full_name);
}
EXPORT_SYMBOL(unflatten_device_tree);

int
of_bus_n_addr_cells(struct device_node *np)
{
    u32 cells;

    for (; np; np = np->parent)
        if (!of_property_read_u32(np, "#address-cells", &cells))
            return cells;

    /* No #address-cells property for the root node */
    return OF_ROOT_NODE_ADDR_CELLS_DEFAULT;
}

int
of_n_addr_cells(struct device_node *np)
{
    if (np->parent)
        np = np->parent;

    return of_bus_n_addr_cells(np);
}
EXPORT_SYMBOL(of_n_addr_cells);

int
of_bus_n_size_cells(struct device_node *np)
{
    u32 cells;

    for (; np; np = np->parent)
        if (!of_property_read_u32(np, "#size-cells", &cells))
            return cells;

    /* No #size-cells property for the root node */
    return OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
}

int of_n_size_cells(struct device_node *np)
{
    if (np->parent)
        np = np->parent;

    return of_bus_n_size_cells(np);
}
EXPORT_SYMBOL(of_n_size_cells);

const struct of_device_id *
of_match_device(const struct of_device_id *matches, const struct device *dev)
{
    if ((!matches) || (!dev->of_node))
        return NULL;
    return of_match_node(matches, dev->of_node);
}
EXPORT_SYMBOL(of_match_device);

const void *of_device_get_match_data(const struct device *dev)
{
    const struct of_device_id *match;

    match = of_match_device(dev->driver->of_match_table, dev);
    if (!match)
        return NULL;

    return match->data;
}
EXPORT_SYMBOL(of_device_get_match_data);

static int
init_module(void)
{
    printk("module[of]: init begin ...\n");
    unflatten_device_tree();
    printk("module[of]: init end!\n");
    return 0;
}
