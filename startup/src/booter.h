// SPDX-License-Identifier: GPL-2.0-only

#ifndef _BOOTER_H_
#define _BOOTER_H_

/*
 * Qemu pflash is used for modules repository
 * Note: we should select the second pflash (unit=1),
 * because the first pflash only acts as BIOS.
 */
#define FLASH_SIZE      0x0000000002000000UL
#define FLASH_PA        0x0000000022000000UL
#define FLASH_VA        FLASH_PA

extern void sbi_puts(const char *s);
extern void sbi_put_u64(unsigned long n);
extern void sbi_put_dec(unsigned long n);

typedef int (*init_module_t)(void);
typedef void (*exit_module_t)(void);

extern int hex_to_str(unsigned long n, char *str, size_t len);
extern int dec_to_str(unsigned long n, char *str, size_t len);

extern void load_modules(void);

//extern void booter_panic(void);
extern void sbi_shutdown(void);

#define booter_panic() \
do { \
    sbi_puts("\n########################\n"); \
    sbi_puts("PANIC: "); \
    sbi_puts(__FUNCTION__); \
    sbi_puts(" ("); \
    sbi_puts(__FILE__); \
    sbi_puts(":"); \
    sbi_put_dec(__LINE__); \
    sbi_puts(")"); \
    sbi_puts("\n########################\n"); \
    sbi_shutdown(); \
} while (0);

#endif /* _BOOTER_H_ */
