// SPDX-License-Identifier: GPL-2.0
/*
 * RISC-V SBI based earlycon
 *
 * Copyright (C) 2018 Anup Patel <anup@brainfault.org>
 */
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <asm/sbi.h>

static void sbi_putc(struct uart_port *port, int c)
{
	sbi_console_putchar(c);
}

static void sbi_console_write(struct console *con,
			      const char *s, unsigned n)
{
	struct earlycon_device *dev = con->data;
	uart_console_write(&dev->port, s, n, sbi_putc);
}

int __init early_sbi_setup(struct earlycon_device *device,
				  const char *opt)
{
	device->con->write = sbi_console_write;
	return 0;
}
EARLYCON_DECLARE(sbi, early_sbi_setup);

// From drivers/tty/serial/serial_core.c
/**
 *  uart_console_write - write a console message to a serial port
 *  @port: the port to write the message
 *  @s: array of characters
 *  @count: number of characters in string to write
 *  @putchar: function to write character to port
 */
void uart_console_write(struct uart_port *port, const char *s,
            unsigned int count,
            void (*putchar)(struct uart_port *, int))
{
    unsigned int i;

    for (i = 0; i < count; i++, s++) {
        if (*s == '\n')
            putchar(port, '\r');
        putchar(port, *s);
    }
}
EXPORT_SYMBOL_GPL(uart_console_write);
