// SPDX-License-Identifier: GPL-2.0+

#include <fs.h>
#include <csr.h>
#include <elf.h>
#include <mman.h>
#include <slab.h>
#include <errno.h>
#include <auxvec.h>
#include <export.h>
#include <string.h>
#include <binfmts.h>
#include <current.h>
#include <uaccess.h>
#include <processor.h>
#include <mman-common.h>
#include <limits.h>
#include <switch_to.h>
#include <payload.h>

#define ELF_MIN_ALIGN       PAGE_SIZE
#define ELF_PAGESTART(_v)   ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v)  ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v)   (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))

#define STACK_ADD(sp, items) ((elf_addr_t *)(sp) - (items))
#define STACK_ROUND(sp, items) (((unsigned long) (sp - items)) &~ 15UL)
#define STACK_ALLOC(sp, len) ({ sp -= len ; sp; })

#define BAD_ADDR(x) (unlikely((unsigned long)(x) >= TASK_SIZE))

static int
elf_read(struct file *file, void *buf, size_t len, loff_t pos)
{
    ssize_t rv;

    rv = kernel_read(file, buf, len, &pos);
    if (unlikely(rv != len)) {
        return (rv < 0) ? rv : -EIO;
    }
    return 0;
}

static struct elf_phdr *
load_elf_phdrs(const struct elfhdr *elf_ex, struct file *elf_file)
{
    int retval;
    unsigned int size;
    struct elf_phdr *elf_phdata = NULL;

    /*
     * If the size of this structure has changed, then punt, since
     * we will be doing the wrong thing.
     */
    if (elf_ex->e_phentsize != sizeof(struct elf_phdr))
        panic("bad phdr!");

    /* Sanity check the number of program headers... */
    /* ...and their total size. */
    size = sizeof(struct elf_phdr) * elf_ex->e_phnum;
    if (size == 0 || size > 65536 || size > ELF_MIN_ALIGN)
        panic("bad size!");

    elf_phdata = kmalloc(size, GFP_KERNEL);
    if (!elf_phdata)
        panic("out of memory!");

    /* Read in the program headers */
    retval = elf_read(elf_file, elf_phdata, size, elf_ex->e_phoff);
    if (retval < 0)
        panic("read elf header error!");

    return elf_phdata;
}

static inline int
make_prot(u32 p_flags)
{
    int prot = 0;

    if (p_flags & PF_R)
        prot |= PROT_READ;
    if (p_flags & PF_W)
        prot |= PROT_WRITE;
    if (p_flags & PF_X)
        prot |= PROT_EXEC;

    return prot;
}

static unsigned long
elf_map(struct file *filep, unsigned long addr,
        const struct elf_phdr *eppnt, int prot, int type,
        unsigned long total_size)
{
    unsigned long map_addr;
    unsigned long size = eppnt->p_filesz + ELF_PAGEOFFSET(eppnt->p_vaddr);
    unsigned long off = eppnt->p_offset - ELF_PAGEOFFSET(eppnt->p_vaddr);

    addr = ELF_PAGESTART(addr);
    size = ELF_PAGEALIGN(size);

    if (!size)
        return addr;

    /*
    * total_size is the size of the ELF (interpreter) image.
    * The _first_ mmap needs to know the full size, otherwise
    * randomization might put this image into an overlapping
    * position with the ELF binary image. (since size < total_size)
    * So we first map the 'big' image - and unmap the remainder at
    * the end. (which unmap is needed for ELF images with holes.)
    */
    if (total_size) {
        total_size = ELF_PAGEALIGN(total_size);
        map_addr = vm_mmap(filep, addr, total_size, prot, type, off);
        if (!BAD_ADDR(map_addr))
            vm_munmap(map_addr+size, total_size-size);
    } else
        map_addr = vm_mmap(filep, addr, size, prot, type, off);

    if ((type & MAP_FIXED_NOREPLACE) &&
        PTR_ERR((void *)map_addr) == -EEXIST)
        panic("elf segment requested but the memory is mapped already");

    return(map_addr);
}

static int set_brk(unsigned long start, unsigned long end, int prot)
{
    start = ELF_PAGEALIGN(start);
    end = ELF_PAGEALIGN(end);
    if (end > start) {
        /*
         * Map the last of the bss segment.
         * If the header is requesting these pages to be
         * executable, honour that (ppc32 needs this).
         */
        int error = vm_brk_flags(start, end - start,
                                 prot & PROT_EXEC ? VM_EXEC : 0);
        if (error)
            return error;
    }
    current->mm->start_brk = current->mm->brk = end;
    return 0;
}

static int load_elf_binary(struct linux_binprm *bprm);

static struct linux_binfmt elf_format = {
    .load_binary = load_elf_binary,
};

static int
create_elf_tables(struct linux_binprm *bprm, const struct elfhdr *exec,
                  unsigned long load_addr,
                  unsigned long interp_load_addr,
                  unsigned long e_entry)
{
    int items;
    int ei_index;
    elf_addr_t *sp;
    elf_addr_t *elf_info;
    elf_addr_t *u_platform;
    elf_addr_t *u_base_platform;
    elf_addr_t *u_rand_bytes;
    struct vm_area_struct *vma;
    unsigned char k_rand_bytes[16];
    int argc = bprm->argc;
    int envc = bprm->envc;
    unsigned long p = bprm->p;
    struct mm_struct *mm = current->mm;

    u_platform = NULL;
    u_base_platform = NULL;

    /*
     * Generate 16 random bytes for userspace PRNG seeding.
     */
    //get_random_bytes(k_rand_bytes, sizeof(k_rand_bytes));
    u_rand_bytes = (elf_addr_t *) STACK_ALLOC(p, sizeof(k_rand_bytes));
    if (copy_to_user(u_rand_bytes, k_rand_bytes, sizeof(k_rand_bytes)))
        return -EFAULT;

    /* Create the ELF interpreter info */
    elf_info = (elf_addr_t *)mm->saved_auxv;
    /* update AT_VECTOR_SIZE_BASE if the number of NEW_AUX_ENT() changes */
#define NEW_AUX_ENT(id, val) \
    do { \
        *elf_info++ = id; \
        *elf_info++ = val; \
    } while (0)

    NEW_AUX_ENT(AT_PAGESZ, ELF_EXEC_PAGESIZE);
    NEW_AUX_ENT(AT_PHDR, load_addr + exec->e_phoff);
    NEW_AUX_ENT(AT_PHENT, sizeof(struct elf_phdr));
    NEW_AUX_ENT(AT_PHNUM, exec->e_phnum);
    NEW_AUX_ENT(AT_BASE, interp_load_addr);
    NEW_AUX_ENT(AT_FLAGS, 0);
    NEW_AUX_ENT(AT_ENTRY, e_entry);
    NEW_AUX_ENT(AT_SECURE, bprm->secureexec);
    NEW_AUX_ENT(AT_RANDOM, (elf_addr_t)(unsigned long)u_rand_bytes);
    NEW_AUX_ENT(AT_EXECFN, bprm->exec);
#undef NEW_AUX_ENT

    /* AT_NULL is zero; clear the rest too */
    memset(elf_info, 0,(char *)mm->saved_auxv +
           sizeof(mm->saved_auxv) - (char *)elf_info);

    /* And advance past the AT_NULL entry.  */
    elf_info += 2;

    ei_index = elf_info - (elf_addr_t *)mm->saved_auxv;
    sp = STACK_ADD(p, ei_index);

    items = (argc + 1) + (envc + 1) + 1;
    bprm->p = STACK_ROUND(sp, items);

    sp = (elf_addr_t *)bprm->p;

    /*
     * Grow the stack manually; some architectures have a limit on how
     * far ahead a user-space access may be in order to grow the stack.
     */
    vma = find_extend_vma(mm, bprm->p);
    if (!vma)
        panic("find extend vma error!");

    /* Now, let's put argc (and argv, envp if appropriate) on the stack */
    if (put_user(argc, sp++))
        panic("put argc to userspace error!");

    /* Populate list of argv pointers back to argv strings. */
    p = mm->arg_end = mm->arg_start;
    while (argc-- > 0) {
        size_t len;
        if (put_user((elf_addr_t)p, sp++))
            panic("put argv to userspace error!");
        len = strnlen_user((void *)p, MAX_ARG_STRLEN);
        if (!len || len > MAX_ARG_STRLEN)
            panic("bad len(%lu)!", len);
        p += len;
    }
    if (put_user(0, sp++))
        return -EFAULT;
    mm->arg_end = p;

    /* Populate list of envp pointers back to envp strings. */
    mm->env_end = mm->env_start = p;
    while (envc-- > 0) {
        size_t len;
        if (put_user((elf_addr_t)p, sp++))
            return -EFAULT;
        len = strnlen_user((void *)p, MAX_ARG_STRLEN);
        if (!len || len > MAX_ARG_STRLEN)
            return -EINVAL;
        p += len;
    }
    if (put_user(0, sp++))
        return -EFAULT;
    mm->env_end = p;

    /* Put the elf_info on the stack in the right place.  */
    if (copy_to_user(sp, mm->saved_auxv, ei_index * sizeof(elf_addr_t)))
        panic("copy to user error!");
    return 0;
}

void start_thread(struct pt_regs *regs,
                  unsigned long pc,
                  unsigned long sp)
{
    regs->status = SR_PIE;
    regs->status |= SR_FS_INITIAL;
    /*
    * Restore the initial value to the FP register
    * before starting the user program.
    */
    fstate_restore(current, regs);
    regs->epc = pc;
    regs->sp = sp;

    set_fs(USER_DS);
}

/* We need to explicitly zero any fractional pages
   after the data section (i.e. bss).  This would
   contain the junk from the file that should not
   be in memory
 */
static int padzero(unsigned long elf_bss)
{
    unsigned long nbyte;

    nbyte = ELF_PAGEOFFSET(elf_bss);
    if (nbyte) {
        nbyte = ELF_MIN_ALIGN - nbyte;
        printk("%s: %lx %lx\n", __func__, elf_bss, nbyte);
        if (clear_user((void *) elf_bss, nbyte))
            return -EFAULT;
    }
    return 0;
}

static unsigned long
total_mapping_size(const struct elf_phdr *cmds, int nr)
{
    int i, first_idx = -1, last_idx = -1;

    for (i = 0; i < nr; i++) {
        if (cmds[i].p_type == PT_LOAD) {
            last_idx = i;
            if (first_idx == -1)
                first_idx = i;
        }
    }
    if (first_idx == -1)
        return 0;

    return cmds[last_idx].p_vaddr + cmds[last_idx].p_memsz -
        ELF_PAGESTART(cmds[first_idx].p_vaddr);
}

/* This is much more generalized than the library routine read function,
   so we keep this separate.  Technically the library read function
   is only provided so that we can read a.out libraries that have
   an ELF header */

/* That's for binfmt_elf_fdpic to deal with */
#ifndef elf_check_fdpic
#define elf_check_fdpic(ex) false
#endif

static unsigned long
load_elf_interp(struct elfhdr *interp_elf_ex,
                struct file *interpreter,
                unsigned long no_base,
                struct elf_phdr *interp_elf_phdata)
{
    struct elf_phdr *eppnt;
    unsigned long load_addr = 0;
    int load_addr_set = 0;
    unsigned long last_bss = 0, elf_bss = 0;
    int bss_prot = 0;
    unsigned long error = ~0UL;
    unsigned long total_size;
    int i;

    /* First of all, some simple consistency checks */
    if (interp_elf_ex->e_type != ET_EXEC &&
        interp_elf_ex->e_type != ET_DYN)
        goto out;
    if (!elf_check_arch(interp_elf_ex) ||
        elf_check_fdpic(interp_elf_ex))
        goto out;
    if (!interpreter->f_op->mmap)
        goto out;

    total_size = total_mapping_size(interp_elf_phdata,
                    interp_elf_ex->e_phnum);
    if (!total_size) {
        error = -EINVAL;
        goto out;
    }

    eppnt = interp_elf_phdata;
    for (i = 0; i < interp_elf_ex->e_phnum; i++, eppnt++) {
        if (eppnt->p_type == PT_LOAD) {
            int elf_type = MAP_PRIVATE | MAP_DENYWRITE;
            int elf_prot = make_prot(eppnt->p_flags);
            unsigned long vaddr = 0;
            unsigned long k, map_addr;

            vaddr = eppnt->p_vaddr;
            if (interp_elf_ex->e_type == ET_EXEC || load_addr_set)
                elf_type |= MAP_FIXED_NOREPLACE;
            else if (no_base && interp_elf_ex->e_type == ET_DYN)
                load_addr = -vaddr;

            map_addr = elf_map(interpreter, load_addr + vaddr,
                               eppnt, elf_prot, elf_type, total_size);
            total_size = 0;
            error = map_addr;
            if (BAD_ADDR(map_addr))
                goto out;

            if (!load_addr_set &&
                interp_elf_ex->e_type == ET_DYN) {
                load_addr = map_addr - ELF_PAGESTART(vaddr);
                load_addr_set = 1;
            }

            /*
             * Check to see if the section's size will overflow the
             * allowed task size. Note that p_filesz must always be
             * <= p_memsize so it's only necessary to check p_memsz.
             */
            k = load_addr + eppnt->p_vaddr;
            if (BAD_ADDR(k) ||
                eppnt->p_filesz > eppnt->p_memsz ||
                eppnt->p_memsz > TASK_SIZE ||
                TASK_SIZE - eppnt->p_memsz < k) {
                error = -ENOMEM;
                goto out;
            }

            /*
             * Find the end of the file mapping for this phdr, and
             * keep track of the largest address we see for this.
             */
            k = load_addr + eppnt->p_vaddr + eppnt->p_filesz;
            if (k > elf_bss)
                elf_bss = k;

            /*
             * Do the same thing for the memory mapping - between
             * elf_bss and last_bss is the bss section.
             */
            k = load_addr + eppnt->p_vaddr + eppnt->p_memsz;
            if (k > last_bss) {
                last_bss = k;
                bss_prot = elf_prot;
            }
        }
    }


    /*
     * Now fill out the bss section: first pad the last page from
     * the file up to the page boundary, and zero it from elf_bss
     * up to the end of the page.
     */
    if (padzero(elf_bss)) {
        error = -EFAULT;
        goto out;
    }
    /*
     * Next, align both the file and mem bss up to the page size,
     * since this is where elf_bss was just zeroed up to, and where
     * last_bss will end after the vm_brk_flags() below.
     */
    elf_bss = ELF_PAGEALIGN(elf_bss);
    last_bss = ELF_PAGEALIGN(last_bss);
    /* Finally, if there is still more bss to allocate, do it. */
    if (last_bss > elf_bss) {
        error = vm_brk_flags(elf_bss, last_bss - elf_bss,
                             bss_prot & PROT_EXEC ? VM_EXEC : 0);
        if (error)
            goto out;
    }

    error = load_addr;
out:
    return error;
}

static void
switch_to_unikernel(unsigned long entry, unsigned long sp)
{
    asm volatile (
        "mv sp, %1\n"
        "jr %0\n"
        :: "r" (entry), "r" (sp)
        : "memory"
    );
}

static int load_elf_binary(struct linux_binprm *bprm)
{
    int i;
    int retval;
    unsigned long error;
    struct pt_regs *regs;
    struct mm_struct *mm;
    unsigned long e_entry;
    unsigned long elf_entry;
    struct elf_phdr *elf_ppnt;
    struct elf_phdr *elf_phdata;
    unsigned long elf_bss, elf_brk;
    unsigned long start_code, end_code, start_data, end_data;
    int bss_prot = 0;
    int load_addr_set = 0;
    unsigned long interp_load_addr = 0;
    unsigned long load_addr = 0;
    int executable_stack = EXSTACK_DEFAULT;
    struct elf_phdr *elf_property_phdata = NULL;
    struct elfhdr *elf_ex = (struct elfhdr *)bprm->buf;
    struct elfhdr *interp_elf_ex = NULL;
    struct file *interpreter = NULL; /* to shut gcc up */
    struct elf_phdr *interp_elf_phdata = NULL;
    unsigned long load_bias = 0;
    unsigned long reloc_func_desc = 0;

    retval = -ENOEXEC;
    /* First of all, some simple consistency checks */
    if (memcmp(elf_ex->e_ident, ELFMAG, SELFMAG) != 0)
        goto out;

    if (elf_ex->e_type != ET_EXEC && elf_ex->e_type != ET_DYN)
        goto out;
    if (!elf_check_arch(elf_ex))
        goto out;
    if (!bprm->file->f_op->mmap)
        goto out;

    elf_phdata = load_elf_phdrs(elf_ex, bprm->file);
    if (!elf_phdata)
        goto out;

    elf_ppnt = elf_phdata;
    for (i = 0; i < elf_ex->e_phnum; i++, elf_ppnt++) {
        char *elf_interpreter;

        if (elf_ppnt->p_type == PT_GNU_PROPERTY) {
            elf_property_phdata = elf_ppnt;
            continue;
        }

        if (elf_ppnt->p_type != PT_INTERP)
            continue;

        /*
         * This is the program interpreter used for shared libraries -
         * for now assume that this is an a.out format binary.
         */
        retval = -ENOEXEC;
        if (elf_ppnt->p_filesz > PATH_MAX || elf_ppnt->p_filesz < 2)
            panic("out_free_ph");

        retval = -ENOMEM;
        elf_interpreter = kmalloc(elf_ppnt->p_filesz, GFP_KERNEL);
        if (!elf_interpreter)
            panic("out_free_ph");

        retval = elf_read(bprm->file, elf_interpreter,
                          elf_ppnt->p_filesz, elf_ppnt->p_offset);
        if (retval < 0)
            panic("out_free_interp");
        /* make sure path is NULL terminated */
        retval = -ENOEXEC;
        if (elf_interpreter[elf_ppnt->p_filesz - 1] != '\0')
            panic("out_free_interp");

        printk("%s: interp[%s]\n", __func__, elf_interpreter);

        /* Todo: implement a MAP in bootrd */
        if (strcmp(kbasename(elf_interpreter),
                   "ld-musl-riscv64.so.1") == 0) {
            strcpy(elf_interpreter, "payload://libc.so");
        }

        interpreter = open_exec(elf_interpreter);
        retval = PTR_ERR(interpreter);
        if (IS_ERR(interpreter))
            panic("out_free_ph");

        /* Todo: wrap a common function */
        {
            #define PAYLOAD_PROTOCOL "payload://"
            extern const struct file_operations payload_file_operations;
            extern struct list_head payloads;

            struct payload *payload;
            list_for_each_entry(payload, &payloads, lh) {
                int prefix_size = strlen(PAYLOAD_PROTOCOL);
                printk("++++++ %s: payload '%s'\n", __func__, payload->name);
                if (strcmp(elf_interpreter + prefix_size, payload->name) == 0) {
                    interpreter->private_data = (void *)payload->ptr;
                    break;
                }
            }
            if (interpreter->private_data == NULL) {
                panic("%s: no payload named '%s'\n",
                      __func__, elf_interpreter);
            }
            interpreter->f_mode |= FMODE_READ;
            interpreter->f_mode |= FMODE_CAN_READ;
            interpreter->f_op = &payload_file_operations;
        }
        kfree(elf_interpreter);

        interp_elf_ex = kmalloc(sizeof(*interp_elf_ex), GFP_KERNEL);
        if (!interp_elf_ex) {
            retval = -ENOMEM;
            panic("out_free_ph");
        }


        /* Get the exec headers */
        retval = elf_read(interpreter, interp_elf_ex,
                          sizeof(*interp_elf_ex), 0);
        if (retval < 0)
            panic("out_free_dentry");

        break;
    }

    elf_ppnt = elf_phdata;
    for (i = 0; i < elf_ex->e_phnum; i++, elf_ppnt++) {
        switch (elf_ppnt->p_type) {
        case PT_GNU_STACK:
            if (elf_ppnt->p_flags & PF_X)
                executable_stack = EXSTACK_ENABLE_X;
            else
                executable_stack = EXSTACK_DISABLE_X;
            break;

        case PT_LOPROC ... PT_HIPROC:
            break;
        }
    }

    /* Some simple consistency checks for the interpreter */
    if (interpreter) {
        retval = -ELIBBAD;
        /* Not an ELF interpreter */
        if (memcmp(interp_elf_ex->e_ident, ELFMAG, SELFMAG) != 0)
            panic("out_free_dentry");
        /* Verify the interpreter has a valid arch */
        if (!elf_check_arch(interp_elf_ex))
            panic("out_free_dentry");

        /* Load the interpreter program headers */
        interp_elf_phdata = load_elf_phdrs(interp_elf_ex, interpreter);
        if (!interp_elf_phdata)
            panic("out_free_dentry");

        /* Pass PT_LOPROC..PT_HIPROC headers to arch code */
        elf_property_phdata = NULL;
        elf_ppnt = interp_elf_phdata;
        for (i = 0; i < interp_elf_ex->e_phnum; i++, elf_ppnt++) {
            switch (elf_ppnt->p_type) {
            case PT_GNU_PROPERTY:
                elf_property_phdata = elf_ppnt;
                break;

            case PT_LOPROC ... PT_HIPROC:
                break;
            }
        }
    }

    retval = begin_new_exec(bprm);
    if (retval)
        panic("begin new exec error!");

    setup_new_exec(bprm);

    /* Do this so that we can load the interpreter, if need be.  We will
       change some of these later */
    retval = setup_arg_pages(bprm, STACK_TOP, executable_stack);
    if (retval < 0)
        panic("setup arg pages error!");

    elf_bss = 0;
    elf_brk = 0;

    start_code = ~0UL;
    end_code = 0;
    start_data = 0;
    end_data = 0;

    /* Now we do a little grungy work by mmapping the ELF image into
       the correct location in memory. */
    for(i = 0, elf_ppnt = elf_phdata;
        i < elf_ex->e_phnum; i++, elf_ppnt++) {
        int elf_prot, elf_flags;
        unsigned long k, vaddr;
        unsigned long total_size = 0;

        if (elf_ppnt->p_type != PT_LOAD)
            continue;

        printk("%s: p_type(%x) p_vaddr(%lx)\n",
               __func__, elf_ppnt->p_type, elf_ppnt->p_vaddr);

        if (unlikely (elf_brk > elf_bss))
            panic("bad elf_brk or elf_bss!");

        elf_prot = make_prot(elf_ppnt->p_flags);

        elf_flags = MAP_PRIVATE | MAP_DENYWRITE | MAP_EXECUTABLE;

        vaddr = elf_ppnt->p_vaddr;

        /*
         * If we are loading ET_EXEC or we have already performed
         * the ET_DYN load_addr calculations, proceed normally.
         */
        if (elf_ex->e_type == ET_EXEC || load_addr_set) {
            elf_flags |= MAP_FIXED;
        } else if (elf_ex->e_type == ET_DYN) {
            if (interpreter) {
                load_bias = ELF_ET_DYN_BASE;
                elf_flags |= MAP_FIXED;
            } else
                load_bias = 0;

            /*
             * Since load_bias is used for all subsequent loading
             * calculations, we must lower it by the first vaddr
             * so that the remaining calculations based on the
             * ELF vaddrs will be correctly offset. The result
             * is then page aligned.
             */
            load_bias = ELF_PAGESTART(load_bias - vaddr);

            total_size = total_mapping_size(elf_phdata, elf_ex->e_phnum);
            if (!total_size) {
                retval = -EINVAL;
                panic("out_free_dentry");
            }
        }

        printk("--- vaddr: %lx, elf_prot: %x, elf_flags: %x, total_size: %lx\n",
               vaddr, elf_prot, elf_flags, total_size);
        error = elf_map(bprm->file, load_bias + vaddr, elf_ppnt,
                        elf_prot, elf_flags, total_size);
        if (BAD_ADDR(error))
            panic("elf map error!");

        if (!load_addr_set) {
            load_addr_set = 1;
            load_addr = (elf_ppnt->p_vaddr - elf_ppnt->p_offset);
            if (elf_ex->e_type == ET_DYN) {
                load_bias += error - ELF_PAGESTART(load_bias + vaddr);
                load_addr += load_bias;
                reloc_func_desc = load_bias;
            }
        }

        k = elf_ppnt->p_vaddr;
        if ((elf_ppnt->p_flags & PF_X) && k < start_code)
            start_code = k;
        if (start_data < k)
            start_data = k;

        /*
         * Check to see if the section's size will overflow the
         * allowed task size. Note that p_filesz must always be
         * <= p_memsz so it is only necessary to check p_memsz.
         */
        if (BAD_ADDR(k) || elf_ppnt->p_filesz > elf_ppnt->p_memsz ||
            elf_ppnt->p_memsz > TASK_SIZE ||
            TASK_SIZE - elf_ppnt->p_memsz < k)
            panic("out free dentry!");

        k = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;

        if (k > elf_bss)
            elf_bss = k;
        if ((elf_ppnt->p_flags & PF_X) && end_code < k)
            end_code = k;
        if (end_data < k)
            end_data = k;
        k = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
        if (k > elf_brk) {
            bss_prot = elf_prot;
            elf_brk = k;
        }
    }

    e_entry = elf_ex->e_entry + load_bias;
    elf_bss += load_bias;
    elf_brk += load_bias;
    start_code += load_bias;
    end_code += load_bias;
    start_data += load_bias;
    end_data += load_bias;

    printk("--- %s: code(%lx, %lx) data(%lx, %lx) bss(%lx) brk(%lx) entry(%lx)\n",
           __func__, start_code, end_code, start_data, end_data,
           elf_bss, elf_brk, e_entry);

    /* Calling set_brk effectively mmaps the pages that we need
     * for the bss and break sections.  We must do this before
     * mapping in the interpreter, to make sure it doesn't wind
     * up getting placed where the bss needs to go.
     */
    retval = set_brk(elf_bss, elf_brk, bss_prot);
    if (retval)
        panic("set brk error!");

    if (likely(elf_bss != elf_brk) && unlikely(padzero(elf_bss)))
        panic("pad zero to bss error!");

    if (interpreter) {
        elf_entry = load_elf_interp(interp_elf_ex,
                                    interpreter,
                                    load_bias,
                                    interp_elf_phdata);
        if (!IS_ERR((void *)elf_entry)) {
            /*
             * load_elf_interp() returns relocation
             * adjustment
             */
            interp_load_addr = elf_entry;
            elf_entry += interp_elf_ex->e_entry;
        }
        if (BAD_ADDR(elf_entry)) {
            retval = IS_ERR((void *)elf_entry) ?
                (int)elf_entry : -EINVAL;
            panic("bad elf_entry for interp!");
        }
        reloc_func_desc = interp_load_addr;

        //allow_write_access(interpreter);
        //fput(interpreter);

        kfree(interp_elf_ex);
        kfree(interp_elf_phdata);
    } else {
        elf_entry = e_entry;
        if (BAD_ADDR(elf_entry))
            panic("bad elf_entry!");
    }

    kfree(elf_phdata);

    set_binfmt(&elf_format);

    retval = create_elf_tables(bprm, elf_ex,
                               load_addr, interp_load_addr, e_entry);
    if (retval < 0)
        panic("create elf tables error!");

    mm = current->mm;
    mm->end_code = end_code;
    mm->start_code = start_code;
    mm->start_data = start_data;
    mm->end_data = end_data;
    mm->start_stack = bprm->p;

    printk("###### %s: switch unikernel entry(%lx) sp(%lx) ######\n",
           __func__, elf_entry, bprm->p);
    switch_to_unikernel(elf_entry, bprm->p);

    panic("###### %s: Stop here! ######\n", __func__);

    regs = current_pt_regs();

    finalize_exec(bprm);
    start_thread(regs, elf_entry, bprm->p);
    retval = 0;

    printk("--- --- %s: step5\n", __func__);
 out:
    return retval;
}

int init_elf_binfmt(void)
{
    register_binfmt(&elf_format);
    return 0;
}

