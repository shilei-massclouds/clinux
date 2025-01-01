// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <tty.h>
#include <slab.h>
#include <errno.h>
#include <major.h>
#include <config.h>
#include <device.h>
#include <export.h>
#include <printk.h>
#include <serial.h>
#include <console.h>
#include <platform.h>

#define UART_NR CONFIG_SERIAL_8250_NR_UARTS

extern void n_tty_init(void);
extern int tty_init(void);

struct of_serial_info {
    int type;
    int line;
};

bool serial_ready;
EXPORT_SYMBOL(serial_ready);

static unsigned int nr_uarts = CONFIG_SERIAL_8250_RUNTIME_UARTS;

static struct uart_8250_port serial8250_ports[UART_NR];

static int
univ8250_console_match(struct console *co,
                       char *name, int idx, char *options)
{
    return -ENODEV;
}

static int
univ8250_console_setup(struct console *co, char *options)
{
    int retval;
    struct uart_port *port;

    /*
     * Check whether an invalid uart number has been specified, and
     * if so, search for the first available port that does have
     * console support.
     */
    if (co->index >= nr_uarts)
        co->index = 0;
    port = &serial8250_ports[co->index].port;
    /* link port to console */
    port->cons = co;

    retval = serial8250_console_setup(port, options, false);
    if (retval != 0)
        port->cons = NULL;
    return retval;
}

static struct uart_driver serial8250_reg;

struct tty_driver *uart_console_device(struct console *co, int *index)
{
    struct uart_driver *p = co->data;
    *index = co->index;
    return p->tty_driver;
}
EXPORT_SYMBOL(uart_console_device);

static struct console univ8250_console = {
    .name   = "ttyS",
    .device = uart_console_device,
    .setup  = univ8250_console_setup,
    .match  = univ8250_console_match,
    .index  = -1,
    .data   = &serial8250_reg,
};

#define SERIAL8250_CONSOLE  (&univ8250_console)

static struct uart_driver serial8250_reg = {
    .driver_name    = "serial",
    .dev_name       = "ttyS",
    .major          = TTY_MAJOR,
    .minor          = 64,
    .cons           = SERIAL8250_CONSOLE,
};

/*
 * This "device" covers _all_ ISA 8250-compatible serial devices listed
 * in the table in include/asm/serial.h
 */
static struct platform_device *serial8250_isa_devs;

static void
uart_configure_port(struct uart_driver *drv, struct uart_state *state,
                    struct uart_port *port)
{
    unsigned int flags;

    printk("%s: 1 mapbase(%lx)\n", __func__, port->mapbase);

    /*
     * If there isn't a port here, don't do anything further.
     */
    if (!port->iobase && !port->mapbase && !port->membase)
        return;

    /*
     * Now do the auto configuration stuff.  Note that config_port
     * is expected to claim the resources and map the port for us.
     */
    flags = 0;

    if (port->flags & UPF_BOOT_AUTOCONF) {
        if (!(port->flags & UPF_FIXED_TYPE)) {
            port->type = PORT_UNKNOWN;
            flags |= UART_CONFIG_TYPE;
        }
        port->ops->config_port(port, flags);
    }

    if (port->type != PORT_UNKNOWN) {

        /*
         * If this driver supports console, and it hasn't been
         * successfully registered yet, try to re-register it.
         * It may be that the port was not available.
         */
        printk("%s: 0\n", __func__);
        if (port->cons && !(port->cons->flags & CON_ENABLED)) {
            register_console(port->cons);
        }
        printk("%s: 1 cons(%lx)\n", __func__, port->cons);
    }
}

int
uart_add_one_port(struct uart_driver *drv, struct uart_port *uport)
{
    int num_groups;
    struct tty_port *port;
    struct device *tty_dev;
    struct uart_state *state;

    state = drv->state + uport->line;
    port = &state->port;

    if (state->uart_port)
        panic("uart_port alreay exists!");

    state->uart_port = uport;
    uport->state = state;

    uport->cons = drv->cons;

    tty_port_link_device(port, drv->tty_driver, uport->line);
    uart_configure_port(drv, state, uport);

    port->console = uart_console(uport);

    num_groups = 2;
    /*
    if (uport->attr_group)
        num_groups++;
        */

    uport->tty_groups = kcalloc(num_groups, sizeof(*uport->tty_groups),
                                GFP_KERNEL);
    if (!uport->tty_groups)
        panic("Out of memory!");

    printk("%s: 1\n", __func__);
    /*
     * Register the port whether it's detected or not.  This allows
     * setserial to be used to alter this port's parameters.
     */
    tty_dev =
        tty_port_register_device_attr_serdev(port, drv->tty_driver,
                                             uport->line, uport->dev,
                                             port, uport->tty_groups);
    printk("%s: 5\n", __func__);
    if (!IS_ERR(tty_dev)) {
        //device_set_wakeup_capable(tty_dev, 1);
    } else {
        panic("Cannot register tty device on line %d\n", uport->line);
    }

    /*
     * Ensure UPF_DEAD is not set.
     */
    uport->flags &= ~UPF_DEAD;
    return 0;
}

static struct uart_8250_port *
serial8250_find_match_or_unused(struct uart_port *port)
{
    int i;

    /* try line number first if still available */
    i = port->line;
    if (i < nr_uarts && serial8250_ports[i].port.type == PORT_UNKNOWN &&
            serial8250_ports[i].port.iobase == 0)
        return &serial8250_ports[i];

    panic("%s: !", __func__);
}

int serial8250_register_8250_port(struct uart_8250_port *up)
{
    struct uart_8250_port *uart;
    int ret = -ENOSPC;

    printk("%s: 1\n", __func__);
    uart = serial8250_find_match_or_unused(&up->port);
    if (uart && uart->port.type != PORT_8250_CIR) {

        uart->port.iobase       = up->port.iobase;
        uart->port.membase      = up->port.membase;
        uart->port.irq          = up->port.irq;
        uart->port.iotype       = up->port.iotype;
        uart->port.flags        = up->port.flags | UPF_BOOT_AUTOCONF;
        uart->port.mapbase      = up->port.mapbase;
        uart->port.mapsize      = up->port.mapsize;

        if (up->port.flags & UPF_FIXED_TYPE)
            uart->port.type = up->port.type;

        if (uart->port.type != PORT_8250_CIR) {
            printk("%s: 2\n", __func__);
            ret = uart_add_one_port(&serial8250_reg, &uart->port);
            if (ret)
                panic("add one port error!");

            ret = uart->port.line;
        } else {
            printk("skipping CIR port at 0x%lx / 0x%lx, IRQ %d\n",
                   uart->port.iobase,
                   (unsigned long long)uart->port.mapbase,
                   uart->port.irq);

            ret = 0;
        }
    }
    return ret;
}

static const struct of_device_id of_platform_serial_table[] = {
    { .compatible = "ns8250",   .data = (void *)PORT_8250, },
    { .compatible = "ns16450",  .data = (void *)PORT_16450, },
    { .compatible = "ns16550a", .data = (void *)PORT_16550A, },
    { .compatible = "ns16550",  .data = (void *)PORT_16550, },
    { .compatible = "ns16750",  .data = (void *)PORT_16750, },
    { .compatible = "ns16850",  .data = (void *)PORT_16850, },
    { /* end of list */ },
};

/*
 * Fill a struct uart_port for a given device node
 */
static int
of_platform_serial_setup(struct platform_device *ofdev,
                         int type, struct uart_8250_port *up,
                         struct of_serial_info *info)
{
    int ret;
    int irq;
    struct resource resource;
    struct uart_port *port = &up->port;
    struct device_node *np = ofdev->dev.of_node;

    ret = of_address_to_resource(np, 0, &resource);
    if (ret)
        panic("invalid address\n");

    port->flags = UPF_SHARE_IRQ | UPF_BOOT_AUTOCONF | UPF_FIXED_PORT |
                  UPF_FIXED_TYPE;

    if (resource_type(&resource) == IORESOURCE_IO) {
        port->iotype = UPIO_PORT;
        port->iobase = resource.start;
    } else {
        port->mapbase = resource.start;
        port->mapsize = resource_size(&resource);

        port->iotype = UPIO_MEM;
        port->flags |= UPF_IOREMAP;
    }

    port->type = type;
    return 0;
}

/*
 * Try to register a serial port
 */
static int
of_platform_serial_probe(struct platform_device *ofdev)
{
    int ret;
    unsigned int port_type;
    struct of_serial_info *info;
    struct uart_8250_port port8250;

    port_type = (unsigned long)of_device_get_match_data(&ofdev->dev);
    if (port_type == PORT_UNKNOWN)
        return -EINVAL;

    info = kzalloc(sizeof(*info), GFP_KERNEL);
    if (info == NULL)
        return -ENOMEM;

    memset(&port8250, 0, sizeof(port8250));
    ret = of_platform_serial_setup(ofdev, port_type, &port8250, info);
    if (ret)
        panic("serial setup error!");

    printk("%s: 2\n", __func__);
    ret = serial8250_register_8250_port(&port8250);
    if (ret < 0)
        panic("bad port!");

    printk("%s: 3\n", __func__);
    info->type = port_type;
    info->line = ret;
    platform_set_drvdata(ofdev, info);
    return 0;
}

static struct platform_driver of_platform_serial_driver = {
    .driver = {
        .name = "of_serial",
        .of_match_table = of_platform_serial_table,
    },
    .probe = of_platform_serial_probe,
};

int
uart_set_options(struct uart_port *port, struct console *co,
                 int baud, int parity, int bits, int flow)
{
    return 0;
}

static void serial8250_isa_init_ports(void)
{
    int i;
    static int first = 1;

    if (!first)
        return;
    first = 0;

    if (nr_uarts > UART_NR)
        nr_uarts = UART_NR;

    printk("%s: 1\n", __func__);
    for (i = 0; i < nr_uarts; i++) {
        struct uart_8250_port *up = &serial8250_ports[i];
        struct uart_port *port = &up->port;

        port->line = i;
        serial8250_init_port(up);
    }
}

int
tty_standard_install(struct tty_driver *driver, struct tty_struct *tty)
{
    driver->ttys[tty->index] = tty;
    return 0;
}
EXPORT_SYMBOL(tty_standard_install);

static int
uart_install(struct tty_driver *driver, struct tty_struct *tty)
{
    struct uart_driver *drv = driver->driver_state;
    struct uart_state *state = drv->state + tty->index;

    tty->driver_data = state;

    return tty_standard_install(driver, tty);
}

static int uart_open(struct tty_struct *tty, struct file *filp)
{
    int retval;
    printk("%s: 1\n", __func__);
    struct uart_state *state = tty->driver_data;

    retval = tty_port_open(&state->port, tty, filp);
    if (retval > 0)
        retval = 0;
    return retval;
}

static int uart_write_room(struct tty_struct *tty)
{
    panic("%s: !", __func__);
}

static void __uart_start(struct tty_struct *tty)
{
    struct uart_state *state = tty->driver_data;
    struct uart_port *port = state->uart_port;

    if (port && !uart_tx_stopped(port))
        port->ops->start_tx(port);
}

static int
uart_write(struct tty_struct *tty, const unsigned char *buf, int count)
{
    int c;
    struct circ_buf *circ;
    struct uart_port *port;
    struct uart_state *state = tty->driver_data;
    int ret = 0;

    /*
     * This means you called this function _after_ the port was
     * closed.  No cookie for you.
     */
    if (!state)
        panic("no state!");

    port = state->uart_port;
    circ = &state->xmit;
    if (!circ->buf)
        panic("bad buffer!");

    while (port) {
        c = CIRC_SPACE_TO_END(circ->head, circ->tail, UART_XMIT_SIZE);
        if (count < c)
            c = count;
        if (c <= 0)
            break;
        memcpy(circ->buf + circ->head, buf, c);
        circ->head = (circ->head + c) & (UART_XMIT_SIZE - 1);
        buf += c;
        count -= c;
        ret += c;
    }

    __uart_start(tty);
    return ret;
}

static const struct tty_operations uart_ops = {
    .install    = uart_install,
    .open       = uart_open,
    .write      = uart_write,
    .write_room = uart_write_room,
};

static int
uart_port_startup(struct tty_struct *tty,
                  struct uart_state *state, int init_hw)
{
    int retval = 0;
    unsigned long page;
    struct uart_port *uport = state->uart_port;

    if (uport->type == PORT_UNKNOWN)
        return 1;

    /*
     * Initialise and allocate the transmit and temporary
     * buffer.
     */
    page = get_zeroed_page(GFP_KERNEL);
    if (!page)
        panic("Out of memory!");

    if (!state->xmit.buf) {
        state->xmit.buf = (unsigned char *) page;
        uart_circ_clear(&state->xmit);
    } else {
        panic("xmit buf already init!");
    }

    retval = uport->ops->startup(uport);
    if (retval)
        panic("cant startup!");

    printk("%s: retval(%d)!\n", __func__, retval);
    return retval;
}

static int
uart_startup(struct tty_struct *tty, struct uart_state *state, int init_hw)
{
    struct tty_port *port = &state->port;
    int retval;

    if (tty_port_initialized(port))
        return 0;

    retval = uart_port_startup(tty, state, init_hw);
    if (retval)
        set_bit(TTY_IO_ERROR, &tty->flags);

    return retval;
}

static int
uart_port_activate(struct tty_port *port, struct tty_struct *tty)
{
    int ret;
    struct uart_state *state = container_of(port, struct uart_state, port);

    /*
     * Start up the serial port.
     */
    ret = uart_startup(tty, state, 0);
    if (ret > 0)
        tty_port_set_active(port, 1);

    printk("%s: !\n", __func__);
    return 0;
}

static const struct tty_port_operations uart_port_ops = {
    .activate   = uart_port_activate,
};

int uart_register_driver(struct uart_driver *drv)
{
    int i;
    struct tty_driver *normal;
    int retval = -ENOMEM;

    /*
     * Maybe we should be using a slab cache for this, especially if
     * we have a large number of ports to handle.
     */
    drv->state = kcalloc(drv->nr, sizeof(struct uart_state), GFP_KERNEL);
    if (!drv->state)
        panic("Out of memory!");

    normal = alloc_tty_driver(drv->nr);
    if (!normal)
        panic("Out of memory!");

    drv->tty_driver = normal;

    normal->driver_name     = drv->driver_name;
    normal->name            = drv->dev_name;
    normal->major           = drv->major;
    normal->minor_start     = drv->minor;
    normal->driver_state    = drv;
    tty_set_operations(normal, &uart_ops);

    /*
     * Initialise the UART state(s).
     */
    for (i = 0; i < drv->nr; i++) {
        struct uart_state *state = drv->state + i;
        struct tty_port *port = &state->port;

        tty_port_init(port);
        port->ops = &uart_port_ops;
    }
    return 0;
}

static void serial8250_init(void)
{
    int ret;

    serial8250_isa_init_ports();

    serial8250_reg.nr = UART_NR;
    ret = uart_register_driver(&serial8250_reg);
    if (ret)
        panic("bad driver!");
}

int
init_module(void)
{
    printk("module[of_serial]: init begin ...\n");

    n_tty_init();
    tty_init();
    serial8250_init();

    platform_driver_register(&of_platform_serial_driver);
    serial_ready = true;

    printk("module[of_serial]: init end!\n");
    return 0;
}
