// SPDX-License-Identifier: GPL-2.0-only

#ifndef _BOOTER_H_
#define _BOOTER_H_

extern void sbi_puts(const char *s);
extern void sbi_put_u64(unsigned long n);
extern void sbi_put_dec(unsigned long n);

typedef int (*init_module_t)(void);
typedef void (*exit_module_t)(void);

extern int hex_to_str(unsigned long n, char *str, size_t len);
extern int dec_to_str(unsigned long n, char *str, size_t len);

extern void sbi_shutdown(void);

#define booter_panic(args...) \
do { \
    sbi_puts("\n########################\n"); \
    sbi_puts("PANIC: "); \
    sbi_puts(__FUNCTION__); \
    sbi_puts(" ("); \
    sbi_puts(__FILE__); \
    sbi_puts(":"); \
    sbi_put_dec(__LINE__); \
    sbi_puts(")\n" args ""); \
    sbi_puts("\n########################\n"); \
    sbi_shutdown(); \
} while (0);

#endif /* _BOOTER_H_ */
