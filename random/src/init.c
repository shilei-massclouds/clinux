// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_random_init(void)
{
    sbi_puts("module[random]: init begin ...\n");
    sbi_puts("module[random]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_random_init);

void kill_fasync(struct fasync_struct **fp, int sig, int band)
{
    booter_panic("No impl in 'lib'.");
}

/*
void sha1_init(__u32 *buf)
{
    booter_panic("No impl in 'lib'.");
}
void chacha_block_generic(u32 *state, u8 *stream, int nrounds)
{
    booter_panic("No impl in 'lib'.");
}
bool capable(int cap)
{
    booter_panic("No impl in 'lib'.");
}
*/
int proc_dostring(struct ctl_table *table, int write,
          void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl in 'lib'.");
}
void generate_random_uuid(unsigned char uuid[16])
{
    booter_panic("No impl in 'lib'.");
}
loff_t noop_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl in 'lib'.");
}
int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
{
    booter_panic("No impl in 'lib'.");
}
/*
void sha1_transform(__u32 *digest, const char *data, __u32 *array)
{
    booter_panic("No impl in 'lib'.");
}
*/
/*
int proc_dointvec(struct ctl_table *table, int write, void *buffer,
          size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl in 'lib'.");
}
*/

