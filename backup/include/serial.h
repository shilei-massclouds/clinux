#ifndef _LINUX_SERIAL_8250_H
#define _LINUX_SERIAL_8250_H

#include <tty.h>
#include <list.h>
#include <circ_buf.h>

#define UART_XMIT_SIZE  PAGE_SIZE

#define UART_TX         0       /* Out: Transmit buffer */

#define UART_LSR        5       /* In:  Line Status Register */
#define UART_LSR_THRE   0x20    /* Transmit-hold-register empty */

#define UART_LSR_BRK_ERROR_BITS 0x1E /* BI, FE, PE, OE bits */

/*
 * These are the supported serial types.
 */
#define PORT_UNKNOWN    0
#define PORT_8250       1
#define PORT_16450      2
#define PORT_16550      3
#define PORT_16550A     4
#define PORT_16650      6
#define PORT_16750      8
#define PORT_16850      12

#define PORT_8250_CIR   23  /* CIR infrared port, has its own driver */

/* Port has hardware-assisted s/w flow control */
#define UPF_SHARE_IRQ       ((upf_t) (1 << 24))
/* The exact UART type is known and should not be probed.  */
#define UPF_FIXED_TYPE      ((upf_t) (1 << 27))
#define UPF_BOOT_AUTOCONF   ((upf_t) (1 << 28))
#define UPF_FIXED_PORT      ((upf_t) (1 << 29))
#define UPF_DEAD            ((upf_t) (1 << 30))
#define UPF_IOREMAP         ((upf_t) (1 << 31))

#define UART_CONFIG_TYPE    (1 << 0)

#define uart_console(port) \
    ((port)->cons && (port)->cons->index == (port)->line)

struct uart_port;

struct serial8250_config {
    const char  *name;
    unsigned short  fifo_size;
    unsigned short  tx_loadsz;
};

struct uart_ops {
    void (*start_tx)(struct uart_port *);
    int (*startup)(struct uart_port *);
    void (*config_port)(struct uart_port *, int);
};

/*
 * This is the state information which is persistent across opens.
 */
struct uart_state {
    struct tty_port     port;
    struct circ_buf     xmit;
    struct uart_port    *uart_port;
};

struct uart_driver {
    const char *driver_name;
    const char *dev_name;

    int major;
    int minor;
    int nr;

    struct console *cons;

    /*
     * these are private; the low level driver should not
     * touch these; they should be initialised to NULL
     */
    struct uart_state *state;
    struct tty_driver *tty_driver;
};

typedef unsigned int upf_t;

#define SERIAL_IO_PORT      0
#define SERIAL_IO_HUB6      1
#define SERIAL_IO_MEM       2
#define SERIAL_IO_MEM32     3
#define SERIAL_IO_AU        4
#define SERIAL_IO_TSI       5
#define SERIAL_IO_MEM32BE   6
#define SERIAL_IO_MEM16     7

#define UPIO_PORT   (SERIAL_IO_PORT)    /* 8b I/O port access */
#define UPIO_PORT   (SERIAL_IO_PORT)    /* 8b I/O port access */
#define UPIO_HUB6   (SERIAL_IO_HUB6)    /* Hub6 ISA card */
#define UPIO_MEM    (SERIAL_IO_MEM)     /* driver-specific */
#define UPIO_MEM32  (SERIAL_IO_MEM32)   /* 32b little endian */
#define UPIO_AU     (SERIAL_IO_AU)      /* Au1x00 and RT288x type IO */
#define UPIO_TSI    (SERIAL_IO_TSI)     /* Tsi108/109 type IO */
#define UPIO_MEM16  (SERIAL_IO_MEM16)   /* 16b little endian */

struct uart_port {
    unsigned long   iobase;     /* in/out[bwl] */
    unsigned char   *membase;   /* read/write[bwl] */
    unsigned int    type;       /* port type */
    struct device   *dev;       /* parent device */
    unsigned int    line;       /* port index */
    struct console  *cons;      /* struct console, if any */
    upf_t           flags;

    unsigned int    irq;        /* irq number */
    unsigned char   iotype;     /* io access style */

    resource_size_t mapbase;    /* for ioremap */
    resource_size_t mapsize;

    unsigned char   regshift;   /* reg offset shift */

    struct uart_state *state;   /* pointer to parent state */

    int (*startup)(struct uart_port *port);
    unsigned int (*serial_in)(struct uart_port *, int);
    void (*serial_out)(struct uart_port *, int, int);

    const struct uart_ops *ops;
    const struct attribute_group **tty_groups;  /* all attributes (serial core use only) */
};

struct uart_8250_port {
    struct uart_port    port;
    unsigned int        tx_loadsz;  /* transmit fifo load size */
    unsigned char       cur_iotype; /* Running I/O type */
#define LSR_SAVE_FLAGS UART_LSR_BRK_ERROR_BITS
    unsigned char       lsr_saved_flags;
};

/*
 * Allocate 8250 platform device IDs.  Nothing is implied by
 * the numbering here, except for the legacy entry being -1.
 */
enum {
    PLAT8250_DEV_LEGACY = -1,
    PLAT8250_DEV_PLATFORM,
    PLAT8250_DEV_PLATFORM1,
    PLAT8250_DEV_PLATFORM2,
    PLAT8250_DEV_FOURPORT,
    PLAT8250_DEV_ACCENT,
    PLAT8250_DEV_BOCA,
    PLAT8250_DEV_EXAR_ST16C554,
    PLAT8250_DEV_HUB6,
    PLAT8250_DEV_AU1X00,
    PLAT8250_DEV_SM501,
};

int serial8250_console_setup(struct uart_port *port,
                             char *options, bool probe);

int uart_set_options(struct uart_port *port, struct console *co,
                     int baud, int parity, int bits, int flow);

void serial8250_init_port(struct uart_8250_port *up);

static inline struct uart_8250_port *up_to_u8250p(struct uart_port *up)
{
    return container_of(up, struct uart_8250_port, port);
}

#define uart_circ_empty(circ)   ((circ)->head == (circ)->tail)
#define uart_circ_clear(circ)   ((circ)->head = (circ)->tail = 0)

static inline int uart_tx_stopped(struct uart_port *port)
{
    return 0;
}

static inline int
serial_in(struct uart_8250_port *up, int offset)
{
    return up->port.serial_in(&up->port, offset);
}

static inline void
serial_out(struct uart_8250_port *up, int offset, int value)
{
    up->port.serial_out(&up->port, offset, value);
}

#endif /* _LINUX_SERIAL_8250_H */
