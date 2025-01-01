// SPDX-License-Identifier: GPL-2.0-only

#include <of.h>
#include <irq.h>
#include <errno.h>
#include <export.h>

static bool
__of_node_is_type(const struct device_node *np, const char *type)
{
    const char *match = __of_get_property(np, "device_type", NULL);

    return np && match && type && !strcmp(match, type);
}

static bool
of_node_name_eq(const struct device_node *np, const char *name)
{
    const char *node_name;
    size_t len;

    if (!np)
        return false;

    node_name = kbasename(np->full_name);
    len = strchrnul(node_name, '@') - node_name;

    return (strlen(name) == len) && (strncmp(node_name, name, len) == 0);
}

struct device_node *of_get_parent(const struct device_node *node)
{
    if (!node)
        return NULL;

    return of_node_get(node->parent);
}
EXPORT_SYMBOL(of_get_parent);

struct device_node *of_find_node_by_phandle(phandle handle)
{
    struct device_node *np = NULL;

    if (!handle)
        return NULL;

    for_each_of_allnodes(np) {
        if (np->phandle == handle &&
            !of_node_check_flag(np, OF_DETACHED)) {
            break;
        }
    }

    of_node_get(np);
    return np;
}
EXPORT_SYMBOL(of_find_node_by_phandle);

struct device_node *__of_find_all_nodes(struct device_node *prev)
{
    struct device_node *np;
    if (!prev) {
        np = of_root;
    } else if (prev->child) {
        np = prev->child;
    } else {
        /* Walk back up looking for a sibling, or the end of the structure */
        np = prev;
        while (np->parent && !np->sibling)
            np = np->parent;
        np = np->sibling; /* Might be null at the end of the tree */
    }
    return np;
}

static int
__of_device_is_compatible(const struct device_node *device,
                          const char *compat,
                          const char *type,
                          const char *name)
{
    struct property *prop;
    const char *cp;
    int index = 0, score = 0;

    /* Compatible match has highest priority */
    if (compat && compat[0]) {
        prop = __of_find_property(device, "compatible", NULL);
        for (cp = of_prop_next_string(prop, NULL); cp;
             cp = of_prop_next_string(prop, cp), index++) {
            if (of_compat_cmp(cp, compat, strlen(compat)) == 0) {
                pr_debug("%s: %s, %s\n", __func__, cp, compat);
                score = INT_MAX/2 - (index << 2);
                break;
            }
        }
        if (!score)
            return 0;
    }

    /* Matching type is better than matching name */
    if (type && type[0]) {
        if (!__of_node_is_type(device, type))
            return 0;
        score += 2;
    }

    /* Matching name is a bit better than not */
    if (name && name[0]) {
        if (!of_node_name_eq(device, name))
            return 0;
        score++;
    }

    return score;
}

static const struct of_device_id *
__of_match_node(const struct of_device_id *matches,
                const struct device_node *node)
{
    const struct of_device_id *best_match = NULL;
    int score, best_score = 0;

    if (!matches)
        return NULL;

    for (;
         matches->name[0] || matches->type[0] || matches->compatible[0];
         matches++) {
        score = __of_device_is_compatible(node, matches->compatible,
                                          matches->type, matches->name);
        if (score > best_score) {
            best_match = matches;
            best_score = score;
        }
    }

    return best_match;
}

const struct of_device_id *
of_match_node(const struct of_device_id *matches,
              const struct device_node *node)
{
    const struct of_device_id *match;
    match = __of_match_node(matches, node);
    return match;
}
EXPORT_SYMBOL(of_match_node);

struct device_node *
of_find_matching_node_and_match(struct device_node *from,
                                const struct of_device_id *matches,
                                const struct of_device_id **match)
{
    unsigned long flags;
    struct device_node *np;
    const struct of_device_id *m;

    if (match)
        *match = NULL;

    for_each_of_allnodes_from(from, np) {
        m = __of_match_node(matches, np);
        if (m && of_node_get(np)) {
            if (match)
                *match = m;
            break;
        }
    }
    of_node_put(from);
    return np;
}
EXPORT_SYMBOL(of_find_matching_node_and_match);

static bool
__of_device_is_available(const struct device_node *device)
{
    const char *status;
    int statlen;

    if (!device)
        return false;

    status = __of_get_property(device, "status", &statlen);
    if (status == NULL)
        return true;

    if (statlen > 0) {
        if (!strcmp(status, "okay") || !strcmp(status, "ok"))
            return true;
    }

    return false;
}

bool
of_device_is_available(const struct device_node *device)
{
    return __of_device_is_available(device);
}
EXPORT_SYMBOL(of_device_is_available);

int
of_phandle_iterator_args(struct of_phandle_iterator *it,
                         uint32_t *args, int size)
{
    int i, count;

    count = it->cur_count;

    BUG_ON(size < count);

    for (i = 0; i < count; i++)
        args[i] = be32_to_cpup(it->cur++);

    return count;
}

static int
__of_parse_phandle_with_args(const struct device_node *np,
                             const char *list_name,
                             const char *cells_name,
                             int cell_count, int index,
                             struct of_phandle_args *out_args)
{
    struct of_phandle_iterator it;
    int rc, cur_index = 0;

    of_for_each_phandle(&it, rc, np, list_name, cells_name, cell_count) {
        if (cur_index == index) {
            if (!it.phandle)
                return -ENOENT;

            if (out_args) {
                int c;

                c = of_phandle_iterator_args(&it, out_args->args,
                                             MAX_PHANDLE_ARGS);
                out_args->np = it.node;
                out_args->args_count = c;
            } else {
                of_node_put(it.node);
            }

            printk("%s: %s %x\n", __func__, np->name, out_args->args[0]);
            /* Found it! return success */
            return 0;
        }

        cur_index++;
    }

    return rc;
}

int
of_parse_phandle_with_args(const struct device_node *np,
                           const char *list_name,
                           const char *cells_name,
                           int index,
                           struct of_phandle_args *out_args)
{
    int cell_count = -1;

    if (index < 0)
        return -EINVAL;

    if (!cells_name)
        cell_count = 0;

    return __of_parse_phandle_with_args(np, list_name, cells_name,
                                        cell_count, index, out_args);
}
EXPORT_SYMBOL(of_parse_phandle_with_args);

int
of_phandle_iterator_init(struct of_phandle_iterator *it,
                         const struct device_node *np,
                         const char *list_name,
                         const char *cells_name,
                         int cell_count)
{
    int size;
    const u32 *list;

    memset(it, 0, sizeof(*it));

    /*
     * one of cell_count or cells_name must be provided to determine the
     * argument length.
     */
    if (cell_count < 0 && !cells_name)
        return -EINVAL;

    list = of_get_property(np, list_name, &size);
    if (!list)
        return -ENOENT;

    printk("%s: %d\n", np->name, size);

    it->cells_name = cells_name;
    it->cell_count = cell_count;
    it->parent = np;
    it->list_end = list + size / sizeof(*list);
    it->phandle_end = list;
    it->cur = list;

    return 0;
}
EXPORT_SYMBOL(of_phandle_iterator_init);

int of_phandle_iterator_next(struct of_phandle_iterator *it)
{
    uint32_t count = 0;

    if (it->node) {
        of_node_put(it->node);
        it->node = NULL;
    }

    if (!it->cur || it->phandle_end >= it->list_end)
        return -ENOENT;

    it->cur = it->phandle_end;

    /* If phandle is 0, then it is an empty entry with no arguments. */
    it->phandle = be32_to_cpup(it->cur++);
    printk("%s: %x %x (%p)\n", __func__,
           it->phandle, be32_to_cpup(it->cur), it->cur);

    if (it->phandle) {
        /*
         * Find the provider node and parse the #*-cells property to
         * determine the argument length.
         */
        it->node = of_find_node_by_phandle(it->phandle);

        if (it->cells_name) {
            if (!it->node) {
                panic("%p: could not find phandle\n", it->parent);
            }

            if (of_property_read_u32(it->node, it->cells_name, &count))
                panic("bad property!");
        } else {
            count = it->cell_count;
        }

        /*
         * Make sure that the arguments actually fit in the remaining
         * property data length
         */
        if (it->cur + count > it->list_end) {
            panic("%p: %s = %d found %d\n",
                  it->parent, it->cells_name, count, it->cell_count);
        }
    }

    it->phandle_end = it->cur + count;
    it->cur_count = count;
    return 0;
}
EXPORT_SYMBOL(of_phandle_iterator_next);
