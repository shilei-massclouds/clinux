/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __ASM_GENERIC_MMAN_H
#define __ASM_GENERIC_MMAN_H

#include <mm.h>
#include <mman-common.h>

#define MAP_SHARED          0x01        /* Share changes */
#define MAP_PRIVATE         0x02        /* Changes are private */
#define MAP_SHARED_VALIDATE 0x03    /* share + validate extension flags */

#define MAP_GROWSDOWN   0x0100      /* stack-like segment */
#define MAP_DENYWRITE   0x0800      /* ETXTBSY */
#define MAP_EXECUTABLE  0x1000      /* mark it as an executable */
#define MAP_LOCKED      0x2000      /* pages are locked */
#define MAP_NORESERVE   0x4000      /* don't check for reservations */

#define MAP_32BIT       0
#define MAP_HUGE_2MB    0
#define MAP_HUGE_1GB    0

/*
 * Optimisation macro.  It is equivalent to:
 *      (x & bit1) ? bit2 : 0
 * but this version is faster.
 * ("bit1" and "bit2" must be single bits)
 */
#define _calc_vm_trans(x, bit1, bit2) \
    ((!(bit1) || !(bit2)) ? 0 : \
     ((bit1) <= (bit2) ? ((x) & (bit1)) * ((bit2) / (bit1)) \
      : ((x) & (bit1)) / ((bit1) / (bit2))))

/*
 * The historical set of flags that all mmap implementations implicitly
 * support when a ->mmap_validate() op is not provided in file_operations.
 */
#define LEGACY_MAP_MASK (MAP_SHARED \
        | MAP_PRIVATE \
        | MAP_FIXED \
        | MAP_ANONYMOUS \
        | MAP_DENYWRITE \
        | MAP_EXECUTABLE \
        | MAP_UNINITIALIZED \
        | MAP_GROWSDOWN \
        | MAP_LOCKED \
        | MAP_NORESERVE \
        | MAP_POPULATE \
        | MAP_NONBLOCK \
        | MAP_STACK \
        | MAP_HUGETLB \
        | MAP_32BIT \
        | MAP_HUGE_2MB \
        | MAP_HUGE_1GB)

/*
 * Combine the mmap "prot" argument into "vm_flags" used internally.
 */
static inline unsigned long
calc_vm_prot_bits(unsigned long prot)
{
    return _calc_vm_trans(prot, PROT_READ,  VM_READ ) |
        _calc_vm_trans(prot, PROT_WRITE, VM_WRITE) |
        _calc_vm_trans(prot, PROT_EXEC,  VM_EXEC);
}

/*
 * Combine the mmap "flags" argument into "vm_flags" used internally.
 */
static inline unsigned long
calc_vm_flag_bits(unsigned long flags)
{
    return _calc_vm_trans(flags, MAP_GROWSDOWN,  VM_GROWSDOWN ) |
           _calc_vm_trans(flags, MAP_DENYWRITE,  VM_DENYWRITE ) |
           _calc_vm_trans(flags, MAP_LOCKED,     VM_LOCKED    ) |
           _calc_vm_trans(flags, MAP_SYNC,       VM_SYNC      );
}

/*
 * This is called from mprotect().  PROT_GROWSDOWN and PROT_GROWSUP have
 * already been masked out.
 *
 * Returns true if the prot flags are valid
 */
static inline bool
arch_validate_prot(unsigned long prot, unsigned long addr)
{
    return (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC | PROT_SEM)) == 0;
}

#endif /* __ASM_GENERIC_MMAN_H */
