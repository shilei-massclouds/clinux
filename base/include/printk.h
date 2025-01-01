/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _PRINTK_H_
#define _PRINTK_H_

#include <acgcc.h>
#include <types.h>
#include <console.h>

#define KERN_SOH    "\001"      /* ASCII Start Of Header */
#define KERN_SOH_ASCII  '\001'

#define _CP_RESET   "\033[0m"
#define _CP_RED     "\033[31;1m"
#define _CP_GREEN   "\033[32;1m"
#define _CP_YELLOW  "\033[33;1m"
#define _CP_BLUE    "\033[34;1m"

#define _COLORED(text, color) _CP_RESET color text _CP_RESET
#define _RED(text)      _COLORED(text, _CP_RED)
#define _GREEN(text)    _COLORED(text, _CP_GREEN)
#define _YELLOW(text)   _COLORED(text, _CP_YELLOW)
#define _BLUE(text)     _COLORED(text, _CP_BLUE)

#ifdef X_DEBUG
#define pr_debug printk
#else
#define pr_debug(msg, ...)
#endif

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

void printk(const char *fmt, ...);

struct console_cmdline
{
    char    name[16];       /* Name of the driver       */
    int     index;          /* Minor dev. to use        */
    bool    user_specified; /* Specified by command line vs. platform */
    char    *options;       /* Options for the driver   */
};

int console_setup(char *param, char *value);

void register_console(struct console *newcon);

struct tty_driver *console_device(int *index);

static inline int printk_get_level(const char *buffer)
{
    if (buffer[0] == KERN_SOH_ASCII && buffer[1]) {
        switch (buffer[1]) {
        case '0' ... '7':
        case 'c':   /* KERN_CONT */
            return buffer[1];
        }
    }
    return 0;
}

#endif /* _PRINTK_H_ */
