// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <bug.h>
#include <fork.h>
#include <errno.h>
#include <sched.h>
#include <limits.h>
#include <printk.h>

/*
 * Boot command-line arguments
 */
#define MAX_INIT_ARGS CONFIG_INIT_ENV_ARG_LIMIT
#define MAX_INIT_ENVS CONFIG_INIT_ENV_ARG_LIMIT

static const char *argv_init[MAX_INIT_ARGS+2] = { "init", NULL, };
const char *envp_init[MAX_INIT_ENVS+2] = { "HOME=/", "TERM=linux", NULL, };

static int run_init_process(const char *init_filename)
{
    const char *const *p;

    argv_init[0] = init_filename;
    printk("Run %s as init process\n", init_filename);
    printk("  with arguments:\n");
    for (p = argv_init; *p; p++)
        printk("    %s\n", *p);
    printk("  with environment:\n");
    for (p = envp_init; *p; p++)
        printk("    %s\n", *p);
    return kernel_execve(init_filename, argv_init, envp_init);
}

static int try_to_run_init_process(const char *init_filename)
{
    int ret;

    ret = run_init_process(init_filename);
    if (ret && ret != -ENOENT) {
        panic("Starting init: %s exists but couldn't execute it (error %d)",
              init_filename, ret);
    }

    return ret;
}

static int kernel_init(void *unused)
{
    if (!try_to_run_init_process("/sbin/init"))
        return 0;

    panic("No working init found.  Try passing init= option to kernel. "
          "See Linux Documentation/admin-guide/init.rst for guidance.");
}

void rest_init(void)
{
    int pid;

    printk("%s: 1\n", __func__);
    pid = kernel_thread(kernel_init, NULL, CLONE_FS);
    printk("%s: 2\n", __func__);

    /*
     * The boot idle thread must execute schedule()
     * at least once to get things moving:
     */
    schedule_preempt_disabled();

    panic("Call into cpu_idle with preempt disabled.");
}

void arch_call_rest_init(void)
{
    rest_init();
}
