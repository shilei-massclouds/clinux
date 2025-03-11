// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <linux/kallsyms.h>
#include <linux/random.h>
#include "../../booter/src/booter.h"

#define CREATE_TRACE_POINTS
#include <trace/events/initcall.h>

extern initcall_entry_t __initcall_start[];
extern initcall_entry_t __initcall0_start[];
extern initcall_entry_t __initcall1_start[];
extern initcall_entry_t __initcall2_start[];
extern initcall_entry_t __initcall3_start[];
extern initcall_entry_t __initcall4_start[];
extern initcall_entry_t __initcall5_start[];
extern initcall_entry_t __initcall6_start[];
extern initcall_entry_t __initcall7_start[];
extern initcall_entry_t __initcall_end[];

static initcall_entry_t *initcall_levels[] __initdata = {
    __initcall0_start,
    __initcall1_start,
    __initcall2_start,
    __initcall3_start,
    __initcall4_start,
    __initcall5_start,
    __initcall6_start,
    __initcall7_start,
    __initcall_end,
};

/* Keep these in sync with initcalls in include/linux/init.h */
static const char *initcall_level_names[] __initdata = {
    "pure",
    "core",
    "postcore",
    "arch",
    "subsys",
    "fs",
    "device",
    "late",
};

static ktime_t initcall_calltime;

#ifdef CONFIG_KALLSYMS
struct blacklist_entry {
    struct list_head next;
    char *buf;
};

static __initdata_or_module LIST_HEAD(blacklisted_initcalls);

static int __init initcall_blacklist(char *str)
{
    char *str_entry;
    struct blacklist_entry *entry;

    /* str argument is a comma-separated list of functions */
    do {
        str_entry = strsep(&str, ",");
        if (str_entry) {
            pr_debug("blacklisting initcall %s\n", str_entry);
            entry = memblock_alloc(sizeof(*entry),
                           SMP_CACHE_BYTES);
            if (!entry)
                panic("%s: Failed to allocate %zu bytes\n",
                      __func__, sizeof(*entry));
            entry->buf = memblock_alloc(strlen(str_entry) + 1,
                            SMP_CACHE_BYTES);
            if (!entry->buf)
                panic("%s: Failed to allocate %zu bytes\n",
                      __func__, strlen(str_entry) + 1);
            strcpy(entry->buf, str_entry);
            list_add(&entry->next, &blacklisted_initcalls);
        }
    } while (str_entry);

    return 0;
}

static bool __init_or_module initcall_blacklisted(initcall_t fn)
{
    struct blacklist_entry *entry;
    char fn_name[KSYM_SYMBOL_LEN];
    unsigned long addr;

    if (list_empty(&blacklisted_initcalls))
        return false;

    addr = (unsigned long) dereference_function_descriptor(fn);
    sprint_symbol_no_offset(fn_name, addr);

    /*
     * fn will be "function_name [module_name]" where [module_name] is not
     * displayed for built-in init functions.  Strip off the [module_name].
     */
    strreplace(fn_name, ' ', '\0');

    list_for_each_entry(entry, &blacklisted_initcalls, next) {
        if (!strcmp(fn_name, entry->buf)) {
            pr_debug("initcall %s blacklisted\n", fn_name);
            return true;
        }
    }

    return false;
}
#endif

static __init_or_module void
trace_initcall_start_cb(void *data, initcall_t fn)
{
    ktime_t *calltime = (ktime_t *)data;

    printk(KERN_DEBUG "calling  %pS @ %i\n", fn, task_pid_nr(current));
    *calltime = ktime_get();
}

static __init_or_module void
trace_initcall_finish_cb(void *data, initcall_t fn, int ret)
{
    ktime_t *calltime = (ktime_t *)data;
    ktime_t delta, rettime;
    unsigned long long duration;

    rettime = ktime_get();
    delta = ktime_sub(rettime, *calltime);
    duration = (unsigned long long) ktime_to_ns(delta) >> 10;
    printk(KERN_DEBUG "initcall %pS returned %d after %lld usecs\n",
         fn, ret, duration);
}


#ifdef TRACEPOINTS_ENABLED
static void __init initcall_debug_enable(void)
{
    int ret;

    ret = register_trace_initcall_start(trace_initcall_start_cb,
                        &initcall_calltime);
    ret |= register_trace_initcall_finish(trace_initcall_finish_cb,
                          &initcall_calltime);
    WARN(ret, "Failed to register initcall tracepoints\n");
}
# define do_trace_initcall_start    trace_initcall_start
# define do_trace_initcall_finish   trace_initcall_finish
#else
static inline void do_trace_initcall_start(initcall_t fn)
{
    if (!initcall_debug)
        return;
    trace_initcall_start_cb(&initcall_calltime, fn);
}
static inline void do_trace_initcall_finish(initcall_t fn, int ret)
{
    if (!initcall_debug)
        return;
    trace_initcall_finish_cb(&initcall_calltime, fn, ret);
}
#endif /* !TRACEPOINTS_ENABLED */

static int __init ignore_unknown_bootoption(char *param, char *val,
                   const char *unused, void *arg)
{
    return 0;
}

int __init_or_module do_one_initcall(initcall_t fn)
{
    int count = preempt_count();
    char msgbuf[64];
    int ret;

    if (initcall_blacklisted(fn))
        return -EPERM;

    printk("=================>\n");
    do_trace_initcall_start(fn);
    ret = fn();
    do_trace_initcall_finish(fn, ret);
    printk("<=================\n");

    msgbuf[0] = 0;

    if (preempt_count() != count) {
        sprintf(msgbuf, "preemption imbalance ");
        preempt_count_set(count);
    }
    if (irqs_disabled()) {
        strlcat(msgbuf, "disabled interrupts ", sizeof(msgbuf));
        local_irq_enable();
    }
    WARN(msgbuf[0], "initcall %pS returned with %s\n", fn, msgbuf);

    add_latent_entropy();
    return ret;
}
EXPORT_SYMBOL(do_one_initcall);

static void __init do_initcall_level(int level, char *command_line)
{
    initcall_entry_t *fn;

    sbi_puts(initcall_level_names[level]);
    sbi_puts("\n");
    parse_args(initcall_level_names[level],
           command_line, __start___param,
           __stop___param - __start___param,
           level, level,
           NULL, ignore_unknown_bootoption);

    trace_initcall_level(initcall_level_names[level]);
    for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++)
        do_one_initcall(initcall_from_entry(fn));
}

void __init do_initcalls(void)
{
    int level;
    sbi_puts("do_initcalls: ...\n");
    size_t len = strlen(saved_command_line) + 1;
    char *command_line;

    command_line = kzalloc(len, GFP_KERNEL);
    if (!command_line)
        panic("%s: Failed to allocate %zu bytes\n", __func__, len);

    for (level = 0; level < ARRAY_SIZE(initcall_levels) - 1; level++) {
        /* Parser modifies command_line, restore it each time */
        strcpy(command_line, saved_command_line);
        do_initcall_level(level, command_line);
    }

    kfree(command_line);
    printk("%s: ========== END!\n", __func__);
}
EXPORT_SYMBOL(do_initcalls);
