/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _ASM_RISCV_MMIO_H
#define _ASM_RISCV_MMIO_H

#include <types.h>

#define readb_cpu(c)    ({ u8  __r = __raw_readb(c); __r; })
#define readw_cpu(c)    ({ u16 __r = (__raw_readw(c)); __r; })
#define readl_cpu(c)    ({ u32 __r = (__raw_readl(c)); __r; })

#define writeb_cpu(v, c)    ((void)__raw_writeb((v), (c)))
#define writew_cpu(v, c)    ((void)__raw_writew((u16)(v), (c)))
#define writel_cpu(v, c)    ((void)__raw_writel((u32)(v), (c)))

#define readb(c)    ({ u8  __v; __v = readb_cpu(c); __v; })
#define readw(c)    ({ u16 __v; __v = readw_cpu(c); __v; })
#define readl(c)    ({ u32 __v; __v = readl_cpu(c); __v; })

#define writeb(v, c)    ({ writeb_cpu((v), (c)); })
#define writew(v, c)    ({ writew_cpu((v), (c)); })
#define writel(v, c)    ({ writel_cpu((v), (c)); })

static inline u8 __raw_readb(const volatile void *addr)
{
    u8 val;

    asm volatile("lb %0, 0(%1)" : "=r" (val) : "r" (addr));
    return val;
}

static inline u16 __raw_readw(const volatile void *addr)
{
    u16 val;

    asm volatile("lh %0, 0(%1)" : "=r" (val) : "r" (addr));
    return val;
}

static inline u32 __raw_readl(const volatile void *addr)
{
    u32 val;

    asm volatile("lw %0, 0(%1)" : "=r" (val) : "r" (addr));
    return val;
}

static inline void __raw_writeb(u8 val, volatile void *addr)
{
    asm volatile("sb %0, 0(%1)" : : "r" (val), "r" (addr));
}

static inline void __raw_writew(u16 val, volatile void *addr)
{
    asm volatile("sh %0, 0(%1)" : : "r" (val), "r" (addr));
}

static inline void __raw_writel(u32 val, volatile void *addr)
{
    asm volatile("sw %0, 0(%1)" : : "r" (val), "r" (addr));
}


#endif /* _ASM_RISCV_MMIO_H */
