// SPDX-License-Identifier: GPL-2.0-or-later

#include <bug.h>
#include <export.h>
#include <printk.h>
#include <params.h>
#include <string.h>

/*
 * Parse a string to get a param value pair.
 * You can use " around spaces, but can't escape ".
 * Hyphens and underscores equivalent in parameter names.
 */
char *
next_arg(char *args, char **param, char **val)
{
    char *next;
    unsigned int i;
    unsigned int equals = 0;
    int in_quote = 0;
    int quoted = 0;

    if (*args == '"') {
        args++;
        in_quote = 1;
        quoted = 1;
    }

    for (i = 0; args[i]; i++) {
        if (isspace(args[i]) && !in_quote)
            break;
        if (equals == 0) {
            if (args[i] == '=')
                equals = i;
        }
        if (args[i] == '"')
            in_quote = !in_quote;
    }

    *param = args;
    if (!equals) {
        *val = NULL;
    } else {
        args[equals] = '\0';
        *val = args + equals + 1;

        /* Don't include quotes in value. */
        if (**val == '"') {
            (*val)++;
            if (args[i-1] == '"')
                args[i-1] = '\0';
        }
    }

    if (quoted && args[i-1] == '"')
        args[i-1] = '\0';

    if (args[i]) {
        args[i] = '\0';
        next = args + i + 1;
    } else
        next = args + i;

    /* Chew up trailing spaces. */
    return skip_spaces(next);
}

int
parse_args(char *args,
           const struct kernel_param *params,
           unsigned int num_params)
{
    int i;
    char *key;
    char *value;

    args = skip_spaces(args);
    while (*args) {
        args = next_arg(args, &key, &value);
        printk("%s: (%s:%s)\n", __func__, key, value);
        for (i = 0; i < num_params; i++) {
            if (!strncmp(params[i].name, key, strlen(params[i].name))) {
                if (params[i].setup_func(key, value))
                    break;
            }
        }
    }
    return 0;
}
EXPORT_SYMBOL(parse_args);
