// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/tcp.h>
#include <net/icmp.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_tcp_ipv4_init(void)
{
    sbi_puts("module[tcp_ipv4]: init begin ...\n");
    sbi_puts("module[tcp_ipv4]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tcp_ipv4_init);

DEFINE_ENABLE_FUNC(tcp_ipv4);

struct tcp_congestion_ops tcp_reno;

struct percpu_counter tcp_sockets_allocated;
EXPORT_SYMBOL(tcp_sockets_allocated);

/*
 * Pressure flag: try to collapse.
 * Technical note: it is used by multiple contexts non atomically.
 * All the __sk_mem_schedule() is of this nature: accounting
 * is strict, actions are advisory and have some latency.
 */
unsigned long tcp_memory_pressure __read_mostly;
EXPORT_SYMBOL_GPL(tcp_memory_pressure);

atomic_long_t tcp_memory_allocated; /* Current allocated memory. */
EXPORT_SYMBOL(tcp_memory_allocated);

struct percpu_counter tcp_orphan_count;
EXPORT_SYMBOL_GPL(tcp_orphan_count);

long sysctl_tcp_mem[3] __read_mostly;
EXPORT_SYMBOL(sysctl_tcp_mem);

DEFINE_STATIC_KEY_FALSE(tcp_tx_delay_enabled);
EXPORT_SYMBOL(tcp_tx_delay_enabled);

const struct icmp_err icmp_err_convert[1];
