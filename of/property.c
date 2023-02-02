// SPDX-License-Identifier: GPL-2.0-only

#include <of.h>
#include <errno.h>
#include <export.h>
#include <kernel.h>

struct property *
__of_find_property(const struct device_node *np, const char *name, int *lenp)
{
    struct property *pp;

    if (!np)
        return NULL;

    for (pp = np->properties; pp; pp = pp->next) {
        if (of_prop_cmp(pp->name, name) == 0) {
            if (lenp)
                *lenp = pp->length;
            break;
        }
    }

    return pp;
}
EXPORT_SYMBOL(__of_find_property);

struct property *
of_find_property(const struct device_node *np, const char *name, int *lenp)
{
    struct property *pp;

    pp = __of_find_property(np, name, lenp);
    return pp;
}
EXPORT_SYMBOL(of_find_property);

static void *
of_find_property_value_of_size(const struct device_node *np,
                               const char *propname,
                               u32 min,
                               u32 max,
                               size_t *len)
{
    struct property *prop = of_find_property(np, propname, NULL);

    if (!prop)
        return ERR_PTR(-EINVAL);
    if (!prop->value)
        return ERR_PTR(-ENODATA);
    if (prop->length < min)
        return ERR_PTR(-EOVERFLOW);
    if (max && prop->length > max)
        return ERR_PTR(-EOVERFLOW);

    if (len)
        *len = prop->length;

    return prop->value;
}

int
of_property_read_u32_index(const struct device_node *np,
                           const char *propname,
                           u32 index, u32 *out_value)
{
    const u32 *val;
    val = of_find_property_value_of_size(np, propname,
                                         ((index + 1) * sizeof(*out_value)),
                                         0, NULL);

    if (IS_ERR(val))
        return PTR_ERR(val);

    *out_value = be32_to_cpup(((u32 *)val) + index);
    return 0;
}
EXPORT_SYMBOL(of_property_read_u32_index);

int
of_property_read_variable_u32_array(const struct device_node *np,
                                    const char *propname,
                                    u32 *out_values,
                                    size_t sz_min,
                                    size_t sz_max)
{
    size_t sz, count;
    const u32 *val;

    val = of_find_property_value_of_size(np, propname,
                                         (sz_min * sizeof(*out_values)),
                                         (sz_max * sizeof(*out_values)),
                                         &sz);

    if (IS_ERR(val))
        return PTR_ERR(val);

    if (!sz_max)
        sz = sz_min;
    else
        sz /= sizeof(*out_values);

    count = sz;
    while (count--)
        *out_values++ = be32_to_cpup(val++);

    return sz;
}
EXPORT_SYMBOL(of_property_read_variable_u32_array);
