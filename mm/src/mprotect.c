// SPDX-License-Identifier: GPL-2.0

#include <mman.h>
#include <syscalls.h>
#include <mman-common.h>

static int
do_mprotect_pkey(unsigned long start, size_t len,
                 unsigned long prot, int pkey)
{
    unsigned long end;
    unsigned long reqprot;
    const int grows = prot & (PROT_GROWSDOWN|PROT_GROWSUP);

    start = untagged_addr(start);

    prot &= ~(PROT_GROWSDOWN|PROT_GROWSUP);
    if (grows == (PROT_GROWSDOWN|PROT_GROWSUP)) /* can't be both */
        return -EINVAL;

    if (start & ~PAGE_MASK)
        return -EINVAL;
    if (!len)
        return 0;
    len = PAGE_ALIGN(len);
    end = start + len;
    if (end <= start)
        return -ENOMEM;
    if (!arch_validate_prot(prot, start))
        return -EINVAL;

    reqprot = prot;

    printk("%s: %lx-%lx prot(%lx). Non-implemented!\n",
           __func__, start, len, prot);

    return 0;
}

static long _do_sys_mprotect(unsigned long start, size_t len,
                             unsigned long prot)
{
    return do_mprotect_pkey(start, len, prot, -1);
}

void init_mprotect(void)
{
    do_sys_mprotect = _do_sys_mprotect; 
}
