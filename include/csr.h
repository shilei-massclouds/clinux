/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_CSR_H
#define _ASM_RISCV_CSR_H

#include <const.h>

#define CSR_SSTATUS     0x100
#define CSR_SIE         0x104
#define CSR_STVEC       0x105
#define CSR_SCOUNTEREN  0x106
#define CSR_SSCRATCH    0x140
#define CSR_SEPC        0x141
#define CSR_SCAUSE      0x142
#define CSR_STVAL       0x143
#define CSR_SIP         0x144
#define CSR_SATP        0x180

#define CSR_IE      CSR_SIE
#define CSR_IP      CSR_SIP
#define CSR_STATUS	CSR_SSTATUS
#define CSR_SCRATCH CSR_SSCRATCH

/* Status register flags */
#define SR_SIE		_AC(0x00000002, UL) /* Supervisor Interrupt Enable */
#define SR_MIE		_AC(0x00000008, UL) /* Machine Interrupt Enable */
#define SR_SPIE		_AC(0x00000020, UL) /* Previous Supervisor IE */
#define SR_MPIE		_AC(0x00000080, UL) /* Previous Machine IE */
#define SR_SPP		_AC(0x00000100, UL) /* Previously Supervisor */
#define SR_MPP		_AC(0x00001800, UL) /* Previously Machine */
#define SR_SUM		_AC(0x00040000, UL) /* Supervisor User Memory Access */

#define SR_FS		_AC(0x00006000, UL) /* Floating-point Status */
#define SR_FS_OFF	_AC(0x00000000, UL)
#define SR_FS_INITIAL	_AC(0x00002000, UL)
#define SR_FS_CLEAN	_AC(0x00004000, UL)
#define SR_FS_DIRTY	_AC(0x00006000, UL)

/* Exception cause high bit - is an interrupt if set */
#define CAUSE_IRQ_FLAG  (_AC(1, UL) << (__riscv_xlen - 1))

#define SATP_M_39   _AC(0x8000000000000000, UL)
/* Todo: generate satp_mode based on CONFIG_VA_BITS in config.h */
#define SATP_MODE   SATP_M_39

/* Interrupt causes (minus the high bit) */
#define IRQ_S_SOFT      1
#define IRQ_M_SOFT      3
#define IRQ_S_TIMER     5
#define IRQ_M_TIMER     7
#define IRQ_S_EXT       9
#define IRQ_M_EXT       11

/* Exception causes */
#define EXC_INST_MISALIGNED     0
#define EXC_INST_ACCESS         1
#define EXC_BREAKPOINT          3
#define EXC_LOAD_ACCESS         5
#define EXC_STORE_ACCESS        7
#define EXC_SYSCALL             8
#define EXC_INST_PAGE_FAULT     12
#define EXC_LOAD_PAGE_FAULT     13
#define EXC_STORE_PAGE_FAULT    15

#define SR_IE       SR_SIE
#define SR_PIE      SR_SPIE
#define SR_PP       SR_SPP
#define RV_IRQ_EXT  IRQ_S_EXT

#ifndef __ASSEMBLY__
#define csr_write(csr, val)                             \
({                                                      \
    unsigned long __v = (unsigned long)(val);           \
    __asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0" \
                          : : "rK" (__v)                \
                          : "memory");                  \
})

#define csr_set(csr, val)                               \
({                                                      \
    unsigned long __v = (unsigned long)(val);           \
    __asm__ __volatile__ ("csrs " __ASM_STR(csr) ", %0" \
                          : : "rK" (__v)                \
                          : "memory");                  \
})

#define csr_clear(csr, val)                             \
({                                                      \
    unsigned long __v = (unsigned long)(val);           \
    __asm__ __volatile__ ("csrc " __ASM_STR(csr) ", %0" \
                          : : "rK" (__v)                \
                          : "memory");                  \
})
#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_CSR_H */
