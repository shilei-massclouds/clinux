// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <sbi.h>
#include <acgcc.h>
#include <errno.h>
#include <string.h>
#include <export.h>

#define PREFIX_MAX      32
#define LOG_LINE_MAX    (1024 - PREFIX_MAX)

/*
 *  Array of consoles built from command line options (console=)
 */

#define MAX_CMDLINECONSOLES 8
static struct console_cmdline console_cmdline[MAX_CMDLINECONSOLES];

int console_set_on_cmdline;

static bool has_preferred_console;
static int preferred_console = -1;
EXPORT_SYMBOL(preferred_console);

struct console *console_drivers;
EXPORT_SYMBOL(console_drivers);

/* Default Log level for console */
int console_loglevel = 7;

static void
pre_print(int level)
{
    if (level >= 0 && level <= 3) {
        sbi_puts(_CP_RESET);
        sbi_puts(_CP_RED);
    } else if (level == 4) {
        sbi_puts(_CP_RESET);
        sbi_puts(_CP_DARKRED);
    } else if (level == 5 || level == 6) {
        sbi_puts(_CP_RESET);
        sbi_puts(_CP_BLUE);
    }
}

static void
post_print(int level)
{
    if (level >= 0 && level <= 6) {
        sbi_puts(_CP_RESET);
    }
}

static void
vprintk_func(const char *fmt, va_list args)
{
    int kern_level;
    int offset = 0;
    static char textbuf[LOG_LINE_MAX];

    vsnprintf(textbuf, sizeof(textbuf), fmt, args);

    if ((kern_level = printk_get_level(textbuf)) >= 0) {
        if (kern_level >= console_loglevel)
            return;

        /* skip log level tag (2 bytes) */
        offset = 2;
    }

    pre_print(kern_level);
    sbi_puts(textbuf + offset);
    post_print(kern_level);
}

void printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk_func(fmt, args);
    va_end(args);
}
EXPORT_SYMBOL(printk);
EXPORT_SYMBOL_ITF(printk, lib);

static int
try_enable_new_console(struct console *newcon, bool user_specified)
{
    int i;
    int err;
    struct console_cmdline *c;

    for (i = 0, c = console_cmdline;
         i < MAX_CMDLINECONSOLES && c->name[0];
         i++, c++) {
        if (c->user_specified != user_specified)
            continue;

        if (!newcon->match ||
            newcon->match(newcon, c->name, c->index, c->options) != 0) {
            /* default matching */
            BUG_ON(sizeof(c->name) != sizeof(newcon->name));
            if (strcmp(c->name, newcon->name) != 0)
                continue;
            if (newcon->index >= 0 &&
                newcon->index != c->index)
                continue;
            if (newcon->index < 0)
                newcon->index = c->index;

            if (newcon->setup &&
                (err = newcon->setup(newcon, c->options)) != 0)
                return err;
        }
        newcon->flags |= CON_ENABLED;
        if (i == preferred_console) {
            newcon->flags |= CON_CONSDEV;
            has_preferred_console = true;
        }
        return 0;
    }

    /*
     * Some consoles, such as pstore and netconsole, can be enabled even
     * without matching. Accept the pre-enabled consoles only when match()
     * and setup() had a chance to be called.
     */
    if (newcon->flags & CON_ENABLED && c->user_specified == user_specified)
        return 0;

    return -ENOENT;
}

int unregister_console(struct console *console)
{
    printk("%sconsole [%s%d] disabled\n",
           (console->flags & CON_BOOT) ? "boot" : "" ,
           console->name, console->index);

    return 0;
}

void register_console(struct console *newcon)
{
    int err;
    struct console *bcon = NULL;

    for_each_console(bcon) {
        if (bcon == newcon)
            panic("console '%s%d' already registered",
                  bcon->name, bcon->index);
    }

    /*
     * before we register a new CON_BOOT console, make sure we don't
     * already have a valid console
     */
    if (newcon->flags & CON_BOOT) {
        for_each_console(bcon) {
            if (!(bcon->flags & CON_BOOT))
                panic("Too late to register bootconsole %s%d",
                      newcon->name, newcon->index);
        }
    }

    if (console_drivers && console_drivers->flags & CON_BOOT)
        bcon = console_drivers;

    if (!has_preferred_console || bcon || !console_drivers)
        has_preferred_console = preferred_console >= 0;

    /*
     *  See if we want to use this console driver. If we
     *  didn't select a console we take the first one
     *  that registers here.
     */
    if (!has_preferred_console) {
        if (newcon->index < 0)
            newcon->index = 0;
        if (newcon->setup == NULL ||
            newcon->setup(newcon, NULL) == 0) {
            newcon->flags |= CON_ENABLED;
            if (newcon->device) {
                newcon->flags |= CON_CONSDEV;
                has_preferred_console = true;
            }
        }
    }

    /* See if this console matches one we selected on the command line */
    err = try_enable_new_console(newcon, true);

    /* If not, try to match against the platform default(s) */
    if (err == -ENOENT)
        err = try_enable_new_console(newcon, false);

    printk("%s: err(%d) newcon->flags(%x)\n", __func__, err, newcon->flags);
    /* printk() messages are not printed to the Braille console. */
    if (err || newcon->flags & CON_BRL)
        return;

    /*
     * If we have a bootconsole, and are switching to a real console,
     * don't print everything out again, since when the boot console, and
     * the real console are the same physical device, it's annoying to
     * see the beginning boot messages twice
     */
    if (bcon && ((newcon->flags & (CON_CONSDEV | CON_BOOT)) == CON_CONSDEV))
        newcon->flags &= ~CON_PRINTBUFFER;

    /*
     *  Put this console in the list - keep the
     *  preferred driver at the head of the list.
     */
    if ((newcon->flags & CON_CONSDEV) || console_drivers == NULL) {
        printk("%s: 1\n", __func__);
        newcon->next = console_drivers;
        console_drivers = newcon;
        if (newcon->next)
            newcon->next->flags &= ~CON_CONSDEV;
        /* Ensure this flag is always set for the head of the list */
        newcon->flags |= CON_CONSDEV;
    } else {
        newcon->next = console_drivers->next;
        console_drivers->next = newcon;
    }

    if (newcon->flags & CON_PRINTBUFFER)
        panic("flags has CON_PRINTBUFFER!");

    /*
     * By unregistering the bootconsoles after we enable the real console
     * we get the "console xxx enabled" message on all the consoles -
     * boot consoles, real consoles, etc - this is to ensure that end
     * users know there might be something in the kernel's log buffer that
     * went to the bootconsole (that they do not see on the real console)
     */
    printk("%sconsole [%s%d] enabled\n",
           (newcon->flags & CON_BOOT) ? "boot" : "" ,
           newcon->name, newcon->index);

    if (bcon && ((newcon->flags & (CON_CONSDEV | CON_BOOT)) == CON_CONSDEV)) {
        /* We need to iterate through all boot consoles, to make
         * sure we print everything out, before we unregister them.
         */
        for_each_console(bcon)
            if (bcon->flags & CON_BOOT)
                unregister_console(bcon);
    }
}
EXPORT_SYMBOL(register_console);

static int
__add_preferred_console(char *name, int idx, char *options,
                        char *brl_options, bool user_specified)
{
    int i;
    struct console_cmdline *c;

    /*
     *  See if this tty is not yet registered, and
     *  if we have a slot free.
     */
    for (i = 0, c = console_cmdline;
         i < MAX_CMDLINECONSOLES && c->name[0];
         i++, c++) {
        if (strcmp(c->name, name) == 0 && c->index == idx) {
            if (!brl_options)
                preferred_console = i;
            if (user_specified)
                c->user_specified = true;
            return 0;
        }
    }
    if (i == MAX_CMDLINECONSOLES)
        return -E2BIG;
    if (!brl_options)
        preferred_console = i;
    strlcpy(c->name, name, sizeof(c->name));
    c->options = options;
    c->user_specified = user_specified;

    c->index = idx;
    printk("%s: [%d] idx(%d) name(%s)\n",
           __func__, i, c->index, c->name);
    return 0;
}

/*
 * Set up a console.  Called via do_early_param() in init/main.c
 * for each "console=" parameter in the boot command line.
 */
int
console_setup(char *param, char *value)
{
    char *s;
    int idx;
    char *options = NULL;
    char buf[sizeof(console_cmdline[0].name) + 4]; /* 4 for "ttyS" */

    if (value[0] == 0)
        return 1;

    /*
     * Decode str into name, index, options.
     */
    if (value[0] >= '0' && value[0] <= '9') {
        strcpy(buf, "ttyS");
        strncpy(buf + 4, value, sizeof(buf) - 5);
    } else {
        strncpy(buf, value, sizeof(buf) - 1);
    }
    buf[sizeof(buf) - 1] = 0;
    options = strchr(value, ',');
    if (options)
        *(options++) = 0;
    for (s = buf; *s; s++)
        if (isdigit(*s) || *s == ',')
            break;
    idx = simple_strtoul(s, NULL, 10);
    *s = 0;

    __add_preferred_console(buf, idx, options, NULL, true);
    console_set_on_cmdline = 1;
    return 1;
}
EXPORT_SYMBOL(console_setup);

/*
 * Return the console tty driver structure and its associated index
 */
struct tty_driver *console_device(int *index)
{
    struct console *c;
    struct tty_driver *driver = NULL;

    for_each_console(c) {
        if (!c->device)
            continue;
        printk("%s: 0\n", __func__);
        driver = c->device(c, index);
        if (driver)
            break;
    }
    printk("%s: !\n", __func__);
    return driver;
}
EXPORT_SYMBOL(console_device);
