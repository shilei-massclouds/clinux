// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <mmio.h>
#include <errno.h>
#include <export.h>
#include <ioport.h>
#include <serial.h>
#include <ioremap.h>

void
serial8250_console_write(struct uart_8250_port *up,
                         const char *s, unsigned int count)
{
    panic("%s: !", __func__);
}

int
serial8250_console_setup(struct uart_port *port, char *options,
                         bool probe)
{
    int ret;
    int baud = 9600;
    int bits = 8;
    int parity = 'n';
    int flow = 'n';

    if (!port->iobase && !port->membase)
        return -ENODEV;

    if (options)
        panic("NOT options parse func!");
    else if (probe)
        panic("NOT support for probe!");

    ret = uart_set_options(port, port->cons, baud, parity, bits, flow);
    if (ret)
        return ret;

    return 0;
}

static unsigned int serial8250_port_size(struct uart_8250_port *pt)
{
    if (pt->port.mapsize)
        return pt->port.mapsize;

    panic("%s: !", __func__);
}

static int serial8250_request_std_resource(struct uart_8250_port *up)
{
    int ret = 0;
    struct uart_port *port = &up->port;
    unsigned int size = serial8250_port_size(up);

    switch (port->iotype) {
    case UPIO_AU:
    case UPIO_TSI:
    case UPIO_MEM32:
    case UPIO_MEM16:
    case UPIO_MEM:
        if (!port->mapbase)
            break;

        if (!request_mem_region(port->mapbase, size, "serial")) {
            ret = -EBUSY;
            break;
        }

        if (port->flags & UPF_IOREMAP) {
            port->membase = ioremap(port->mapbase, size);
            if (!port->membase) {
                panic("ioremap error!");
                ret = -ENOMEM;
            }
        }
        break;
    default:
        panic("%s: iotype(%d)!", __func__, port->iotype);
    }

    return ret;
}

static unsigned int mem_serial_in(struct uart_port *p, int offset)
{
    offset = offset << p->regshift;
    return readb(p->membase + offset);
}

static void mem_serial_out(struct uart_port *p, int offset, int value)
{
    offset = offset << p->regshift;
    writeb(value, p->membase + offset);
}

static void set_io_from_upio(struct uart_port *p)
{
    struct uart_8250_port *up = up_to_u8250p(p);

    switch (p->iotype) {
    case UPIO_MEM:
        p->serial_in = mem_serial_in;
        p->serial_out = mem_serial_out;
        break;
    default:
        panic("bad iotype(%u)", p->iotype);
    }
    /* Remember loaded iotype */
    up->cur_iotype = p->iotype;
}

static void serial8250_config_port(struct uart_port *port, int flags)
{
    int ret;
    struct uart_8250_port *up = up_to_u8250p(port);

    printk("%s: 1\n", __func__);

    /*
     * Find the region that we can probe for.  This in turn
     * tells us whether we can probe for the type of port.
     */
    ret = serial8250_request_std_resource(up);
    if (ret < 0)
        panic("bad resource!");

    if (port->iotype != up->cur_iotype)
        set_io_from_upio(port);

    printk("%s: 2 ret(%d)\n", __func__, ret);
}

/*
 * Here we define the default xmit fifo size used for each type of UART.
 */
static const struct serial8250_config uart_config[] = {
    [PORT_UNKNOWN] = {
        .name       = "unknown",
        .fifo_size  = 1,
        .tx_loadsz  = 1,
    },
    [PORT_8250] = {
        .name       = "8250",
        .fifo_size  = 1,
        .tx_loadsz  = 1,
    },
    [PORT_16450] = {
        .name       = "16450",
        .fifo_size  = 1,
        .tx_loadsz  = 1,
    },
    [PORT_16550] = {
        .name       = "16550",
        .fifo_size  = 1,
        .tx_loadsz  = 1,
    },
    [PORT_16550A] = {
        .name       = "16550A",
        .fifo_size  = 16,
        .tx_loadsz  = 16,
    },
};

int serial8250_do_startup(struct uart_port *port)
{
    struct uart_8250_port *up = up_to_u8250p(port);

    printk("%s: 1 port(%d) tx_loadsz(%d)!\n",
           __func__, port->type, up->tx_loadsz);
    if (!up->tx_loadsz)
        up->tx_loadsz = uart_config[port->type].tx_loadsz;

    if (port->iotype != up->cur_iotype)
        set_io_from_upio(port);

    printk("%s: 2 tx_loadsz(%d)!\n", __func__, up->tx_loadsz);
    return 0;
}

static int serial8250_startup(struct uart_port *port)
{
    if (port->startup)
        return port->startup(port);
    return serial8250_do_startup(port);
}

void serial8250_tx_chars(struct uart_8250_port *up)
{
    int count;
    struct uart_port *port = &up->port;
    struct circ_buf *xmit = &port->state->xmit;

    if (uart_circ_empty(xmit))
        return;

    count = up->tx_loadsz;
    printk("%s: send count(%d) ...\n", __func__, count);
    do {
        serial_out(up, UART_TX, xmit->buf[xmit->tail]);
        xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        if (uart_circ_empty(xmit))
            break;
    } while (--count > 0);
}

static inline void __start_tx(struct uart_port *port)
{
    unsigned char lsr;
    struct uart_8250_port *up = up_to_u8250p(port);

    lsr = serial_in(up, UART_LSR);
    up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
    if (lsr & UART_LSR_THRE)
        serial8250_tx_chars(up);
}

static void serial8250_start_tx(struct uart_port *port)
{
    __start_tx(port);
}

static const struct uart_ops serial8250_pops = {
    .start_tx       = serial8250_start_tx,
    .startup        = serial8250_startup,
    .config_port    = serial8250_config_port,
};

void serial8250_init_port(struct uart_8250_port *up)
{
    struct uart_port *port = &up->port;

    port->ops = &serial8250_pops;
}

/**
 *  tty_port_tty_set    -   set the tty of a port
 *  @port: tty port
 *  @tty: the tty
 *
 *  Associate the port and tty pair. Manages any internal refcounts.
 *  Pass NULL to deassociate a port
 */
void tty_port_tty_set(struct tty_port *port, struct tty_struct *tty)
{
    unsigned long flags;

    port->tty = tty;
}
EXPORT_SYMBOL(tty_port_tty_set);

int
tty_port_block_til_ready(struct tty_port *port,
                         struct tty_struct *tty, struct file *filp)
{
    printk("%s: 1\n", __func__);
    return 0;
}

int
tty_port_open(struct tty_port *port,
              struct tty_struct *tty,
              struct file *filp)
{
    printk("%s: 1\n", __func__);
    tty_port_tty_set(port, tty);
    printk("%s: 2\n", __func__);

    if (!tty_port_initialized(port)) {
        clear_bit(TTY_IO_ERROR, &tty->flags);
        if (port->ops->activate) {
            int retval = port->ops->activate(port, tty);
            if (retval)
                return retval;
        }
        tty_port_set_initialized(port, 1);
    }
    printk("%s: 3\n", __func__);
    return tty_port_block_til_ready(port, tty, filp);
}
EXPORT_SYMBOL(tty_port_open);

void tty_port_init(struct tty_port *port)
{
    memset(port, 0, sizeof(*port));
}
EXPORT_SYMBOL(tty_port_init);
