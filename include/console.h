#ifndef _LINUX_CONSOLE_H_
#define _LINUX_CONSOLE_H_

#define CON_PRINTBUFFER (1)
#define CON_CONSDEV     (2)     /* Preferred console, /dev/console */
#define CON_ENABLED     (4)
#define CON_BOOT        (8)
#define CON_BRL         (32)    /* Used for a braille device */
#define CON_ANYTIME     (16)    /* Safe to call when cpu is offline */

struct console {
    char    name[16];
    void    (*write)(struct console *, const char *, unsigned);
    struct tty_driver *(*device)(struct console *, int *);
    int (*setup)(struct console *, char *);
    int (*match)(struct console *, char *name, int idx, char *options);
    short   flags;
    short   index;
    //int cflag;
    void    *data;
    struct   console *next;
};

/*
 * for_each_console() allows you to iterate on each console
 */
#define for_each_console(con) \
    for (con = console_drivers; con != NULL; con = con->next)

extern struct console *console_drivers;

#endif /* _LINUX_CONSOLE_H */
