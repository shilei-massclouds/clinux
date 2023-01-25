/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BINFMTS_H
#define _LINUX_BINFMTS_H

#include <list.h>
#include <resource.h>

#define MAX_ARG_STRLEN  (PAGE_SIZE * 32)
#define MAX_ARG_STRINGS 0x7FFFFFFF

/* sizeof(linux_binprm->buf) */
#define BINPRM_BUF_SIZE 256

/* Stack area protections */
#define EXSTACK_DEFAULT   0 /* Whatever the arch defaults to */
#define EXSTACK_DISABLE_X 1 /* Disable executable stacks */
#define EXSTACK_ENABLE_X  2 /* Enable executable stacks */

/*
 * This structure is used to hold the arguments that are used when loading binaries.
 */
struct linux_binprm {
    struct vm_area_struct *vma;
    unsigned long vma_pages;
    struct mm_struct *mm;
    unsigned long p;        /* current top of mem */

    unsigned int
        /* Should an execfd be passed to userspace? */
        have_execfd:1,

        /* Use the creds of a script (see binfmt_misc) */
        execfd_creds:1,
        /*
         * Set by bprm_creds_for_exec hook to indicate a
         * privilege-gaining exec has happened. Used to set
         * AT_SECURE auxv for glibc.
         */
        secureexec:1,
        /*
         * Set when errors can no longer be returned to the
         * original userspace.
         */
        point_of_no_return:1;

    unsigned long argmin;   /* rlimit marker for copy_strings() */

    struct file *interpreter;
    struct file *file;

    int argc, envc;
    const char *filename;   /* Name of binary as seen by procps */
    const char *fdpath;     /* generated filename for execveat */
    const char *interp;     /* Name of the binary really executed.
                               Most of the time same as filename, but could be
                               different for binfmt_{misc,script} */

    unsigned long loader, exec;

    struct rlimit rlim_stack; /* Saved RLIMIT_STACK used during exec. */

    char buf[BINPRM_BUF_SIZE];
};

/*
 * This structure defines the functions that are used to load the binary formats that
 * linux accepts.
 */
struct linux_binfmt {
    struct list_head lh;
    int (*load_binary)(struct linux_binprm *);
};

void __register_binfmt(struct linux_binfmt *fmt, int insert);

/* Registration of default binfmt handlers */
static inline void register_binfmt(struct linux_binfmt *fmt)
{
    __register_binfmt(fmt, 0);
}

#endif /* _LINUX_BINFMTS_H */
