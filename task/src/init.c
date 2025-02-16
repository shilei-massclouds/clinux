// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/proc_ns.h>
#include <linux/ipc_namespace.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"
#include "../../early_sched/src/sched.h"

static void
setup_task(void)
{
    static bool inited = false;
    if (!inited) {
        riscv_current_is_tp = &init_task;
        init_task.thread_info.cpu = 0;
        inited = true;
    }
}

int
cl_task_init(void)
{
    sbi_puts("module[task]: init begin ...\n");
    setup_task();
    sbi_puts("module[task]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_task_init);

DEFINE_ENABLE_FUNC(task);

void set_task_stack_end_magic(struct task_struct *tsk)
{
    unsigned long *stackend;

    stackend = end_of_stack(tsk);
    *stackend = STACK_END_MAGIC;    /* for overflow detection */
}
EXPORT_SYMBOL(set_task_stack_end_magic);

long do_no_restart_syscall(struct restart_block *param)
{
    return -EINTR;
}

/* to be mentioned only in INIT_TASK */
struct fs_struct init_fs = {
    .users      = 1,
    .lock       = __SPIN_LOCK_UNLOCKED(init_fs.lock),
    .seq        = SEQCNT_SPINLOCK_ZERO(init_fs.seq, &init_fs.lock),
    .umask      = 0022,
};

#ifdef CONFIG_CGROUP_SCHED
/*
 * Default task group.
 * Every task in system belongs to this group at bootup.
 */
struct task_group root_task_group;
EXPORT_SYMBOL(root_task_group);
LIST_HEAD(task_groups);
EXPORT_SYMBOL(task_groups);

/* Cacheline aligned slab cache for task_group */
//static struct kmem_cache *task_group_cache __read_mostly;
#endif

struct pid init_struct_pid = {
    .count      = REFCOUNT_INIT(1),
    .tasks      = {
        { .first = NULL },
        { .first = NULL },
        { .first = NULL },
    },
    .level      = 0,
    .numbers    = { {
        .nr     = 0,
        .ns     = &init_pid_ns,
    }, }
};

/*
 * PID-map pages start out as NULL, they get allocated upon
 * first use and are never deallocated. This way a low pid_max
 * value does not cause lots of bitmaps to be allocated, but
 * the scheme scales to up to 4 million PIDs, runtime.
 */
struct pid_namespace init_pid_ns = {
    .kref = KREF_INIT(2),
    .idr = IDR_INIT(init_pid_ns.idr),
    .pid_allocated = PIDNS_ADDING,
    .level = 0,
    .child_reaper = &init_task,
    .user_ns = &init_user_ns,
    .ns.inum = PROC_PID_INIT_INO,
#ifdef CONFIG_PID_NS
    .ns.ops = &pidns_operations,
#endif
};
EXPORT_SYMBOL_GPL(init_pid_ns);

struct nsproxy init_nsproxy = {
    .count          = ATOMIC_INIT(1),
    .uts_ns         = &init_uts_ns,
#if defined(CONFIG_POSIX_MQUEUE) || defined(CONFIG_SYSVIPC)
    .ipc_ns         = &init_ipc_ns,
#endif
    .mnt_ns         = NULL,
    .pid_ns_for_children    = &init_pid_ns,
#ifdef CONFIG_NET
    .net_ns         = &init_net,
#endif
#ifdef CONFIG_CGROUPS
    .cgroup_ns      = &init_cgroup_ns,
#endif
#ifdef CONFIG_TIME_NS
    .time_ns        = &init_time_ns,
    .time_ns_for_children   = &init_time_ns,
#endif
};

/*
 * The next 2 defines are here bc this is the only file
 * compiled when either CONFIG_SYSVIPC and CONFIG_POSIX_MQUEUE
 * and not CONFIG_IPC_NS.
 */
struct ipc_namespace init_ipc_ns = {
    .count      = REFCOUNT_INIT(1),
    .user_ns = &init_user_ns,
    .ns.inum = PROC_IPC_INIT_INO,
#ifdef CONFIG_IPC_NS
    .ns.ops = &ipcns_operations,
#endif
};

/* cgroup namespace for init task */
struct cgroup_namespace init_cgroup_ns = {
    .count      = REFCOUNT_INIT(2),
    .user_ns    = &init_user_ns,
    .ns.ops     = &cgroupns_operations,
    .ns.inum    = PROC_CGROUP_INIT_INO,
    .root_cset  = &init_css_set,
};

/*
 * The default css_set - used by init and its children prior to any
 * hierarchies being mounted. It contains a pointer to the root state
 * for each subsystem. Also used to anchor the list of css_sets. Not
 * reference-counted, to improve performance when child cgroups
 * haven't been created.
 */
struct css_set init_css_set = {
    .refcount       = REFCOUNT_INIT(1),
    .dom_cset       = &init_css_set,
    .tasks          = LIST_HEAD_INIT(init_css_set.tasks),
    .mg_tasks       = LIST_HEAD_INIT(init_css_set.mg_tasks),
    .dying_tasks        = LIST_HEAD_INIT(init_css_set.dying_tasks),
    .task_iters     = LIST_HEAD_INIT(init_css_set.task_iters),
    .threaded_csets     = LIST_HEAD_INIT(init_css_set.threaded_csets),
    .cgrp_links     = LIST_HEAD_INIT(init_css_set.cgrp_links),
    .mg_preload_node    = LIST_HEAD_INIT(init_css_set.mg_preload_node),
    .mg_node        = LIST_HEAD_INIT(init_css_set.mg_node),

    /*
     * The following field is re-initialized when this cset gets linked
     * in cgroup_init().  However, let's initialize the field
     * statically too so that the default cgroup can be accessed safely
     * early during boot.
     */
    .dfl_cgrp       = &cgrp_dfl_root.cgrp,
};
EXPORT_SYMBOL(init_css_set);

#ifdef CONFIG_KEYS
static struct key_tag init_net_key_domain = { .usage = REFCOUNT_INIT(1) };
#endif

struct net init_net = {
    .count      = REFCOUNT_INIT(1),
    .dev_base_head  = LIST_HEAD_INIT(init_net.dev_base_head),
#ifdef CONFIG_KEYS
    .key_domain = &init_net_key_domain,
#endif
};
EXPORT_SYMBOL(init_net);

static DEFINE_PER_CPU(struct cgroup_rstat_cpu, cgrp_dfl_root_rstat_cpu);

/* the default hierarchy */
struct cgroup_root cgrp_dfl_root = { .cgrp.rstat_cpu = &cgrp_dfl_root_rstat_cpu };
EXPORT_SYMBOL_GPL(cgrp_dfl_root);

struct files_struct init_files = {
    .count      = ATOMIC_INIT(1),
    .fdt        = &init_files.fdtab,
    .fdtab      = {
        .max_fds    = NR_OPEN_DEFAULT,
        .fd     = &init_files.fd_array[0],
        .close_on_exec  = init_files.close_on_exec_init,
        .open_fds   = init_files.open_fds_init,
        .full_fds_bits  = init_files.full_fds_bits_init,
    },
    .file_lock  = __SPIN_LOCK_UNLOCKED(init_files.file_lock),
    .resize_wait    = __WAIT_QUEUE_HEAD_INITIALIZER(init_files.resize_wait),
};

/* init to 2 - one for init_task, one to ensure it is never freed */
struct group_info init_groups = { .usage = ATOMIC_INIT(2) };
EXPORT_SYMBOL(init_groups);

/*
 * The initial credentials for the initial task
 */
struct cred init_cred = {
    .usage          = ATOMIC_INIT(4),
#ifdef CONFIG_DEBUG_CREDENTIALS
    .subscribers        = ATOMIC_INIT(2),
    .magic          = CRED_MAGIC,
#endif
    .uid            = GLOBAL_ROOT_UID,
    .gid            = GLOBAL_ROOT_GID,
    .suid           = GLOBAL_ROOT_UID,
    .sgid           = GLOBAL_ROOT_GID,
    .euid           = GLOBAL_ROOT_UID,
    .egid           = GLOBAL_ROOT_GID,
    .fsuid          = GLOBAL_ROOT_UID,
    .fsgid          = GLOBAL_ROOT_GID,
    .securebits     = SECUREBITS_DEFAULT,
    .cap_inheritable    = CAP_EMPTY_SET,
    .cap_permitted      = CAP_FULL_SET,
    .cap_effective      = CAP_FULL_SET,
    .cap_bset       = CAP_FULL_SET,
    .user           = INIT_USER,
    .user_ns        = &init_user_ns,
    .group_info     = &init_groups,
};
EXPORT_SYMBOL(init_cred);

/* root_user.__count is 1, for init task cred */
struct user_struct root_user = {
    .__count    = REFCOUNT_INIT(1),
    .processes  = ATOMIC_INIT(1),
    .sigpending = ATOMIC_INIT(0),
    .locked_shm     = 0,
    .uid        = GLOBAL_ROOT_UID,
    .ratelimit  = RATELIMIT_STATE_INIT(root_user.ratelimit, 0, 0),
};
EXPORT_SYMBOL(root_user);

// From kernel/fork.c
__cacheline_aligned DEFINE_RWLOCK(tasklist_lock);  /* outer */
EXPORT_SYMBOL(tasklist_lock);
