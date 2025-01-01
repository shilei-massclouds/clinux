// SPDX-License-Identifier: GPL-2.0-only
#include <types.h>
#include <export.h>
#include <string.h>

enum sbi_ext_id {
    SBI_EXT_0_1_SET_TIMER = 0x0,
    SBI_EXT_0_1_CONSOLE_PUTCHAR = 0x1,
    SBI_EXT_0_1_CONSOLE_GETCHAR = 0x2,
    SBI_EXT_0_1_CLEAR_IPI = 0x3,
    SBI_EXT_0_1_SEND_IPI = 0x4,
    SBI_EXT_0_1_REMOTE_FENCE_I = 0x5,
    SBI_EXT_0_1_REMOTE_SFENCE_VMA = 0x6,
    SBI_EXT_0_1_REMOTE_SFENCE_VMA_ASID = 0x7,
    SBI_EXT_0_1_SHUTDOWN = 0x8,
    SBI_EXT_BASE = 0x10,
    SBI_EXT_TIME = 0x54494D45,
    SBI_EXT_IPI = 0x735049,
    SBI_EXT_RFENCE = 0x52464E43,
    SBI_EXT_HSM = 0x48534D,
    SBI_EXT_SRST = 0x53525354,
    SBI_EXT_PMU = 0x504D55,
};

enum sbi_ext_srst_fid {
    SBI_EXT_SRST_RESET = 0,
};

enum sbi_srst_reset_type {
    SBI_SRST_RESET_TYPE_SHUTDOWN = 0,
    SBI_SRST_RESET_TYPE_COLD_REBOOT,
    SBI_SRST_RESET_TYPE_WARM_REBOOT,
};

enum sbi_srst_reset_reason {
    SBI_SRST_RESET_REASON_NONE = 0,
    SBI_SRST_RESET_REASON_SYS_FAILURE,
};

struct sbiret {
    long error;
    long value;
};

struct sbiret
sbi_ecall(int ext, int fid,
          unsigned long arg0, unsigned long arg1, unsigned long arg2,
          unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
    struct sbiret ret;

    register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0);
    register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);
    register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);
    register uintptr_t a3 asm ("a3") = (uintptr_t)(arg3);
    register uintptr_t a4 asm ("a4") = (uintptr_t)(arg4);
    register uintptr_t a5 asm ("a5") = (uintptr_t)(arg5);
    register uintptr_t a6 asm ("a6") = (uintptr_t)(fid);
    register uintptr_t a7 asm ("a7") = (uintptr_t)(ext);
    asm volatile ("ecall"
                  : "+r" (a0), "+r" (a1)
                  : "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6), "r" (a7)
                  : "memory");
    ret.error = a0;
    ret.value = a1;

    return ret;
}
EXPORT_SYMBOL(sbi_ecall);

void sbi_putchar(int ch)
{
    sbi_ecall(SBI_EXT_0_1_CONSOLE_PUTCHAR, 0, ch, 0, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sbi_putchar);

void sbi_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            sbi_putchar('\r');
        sbi_putchar(*s);
    }
}
EXPORT_SYMBOL(sbi_puts);

void sbi_put_u64(unsigned long n)
{
    char buf[UL_STR_SIZE];
    ul_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_u64);

static void sbi_srst_reset(unsigned long type,
                           unsigned long reason)
{
    sbi_ecall(SBI_EXT_SRST, SBI_EXT_SRST_RESET,
              type, reason, 0, 0, 0, 0);
}

void sbi_srst_power_off(void)
{
    sbi_srst_reset(SBI_SRST_RESET_TYPE_SHUTDOWN,
                   SBI_SRST_RESET_REASON_NONE);
}
EXPORT_SYMBOL(sbi_srst_power_off);
