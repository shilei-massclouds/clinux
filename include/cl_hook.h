// SPDX-License-Identifier: GPL-2.0-only

#ifndef _CL_HOOK_H_
#define _CL_HOOK_H_

/*
#define DECLARE_HOOK(rtype, name, args...)  \
    extern rtype (*cl_##name##_hook)(args);

#define DEFINE_HOOK(rtype, name, args...)  \
    rtype (*cl_##name##_hook)(args); \
    EXPORT_SYMBOL(cl_##name##_hook);

#define REGISTER_HOOK(from, to)  \
    cl_##to##_hook = from;

#define HAS_HOOK(name)  \
    (cl_##name##_hook != NULL)

#define HOOK_PANIC(name) \
    do { \
        sbi_puts("************************\nBad Hook: "); \
        sbi_puts(#name); \
        sbi_puts("\n************************\n"); \
        sbi_shutdown(); \
    } while (0)

#define INVOKE_HOOK(name, args...)  \
    do { \
        if (HAS_HOOK(name)) { \
            cl_##name##_hook(args); \
        } else { \
            HOOK_PANIC(name); \
        } \
    } while (0)

#define INVOKE_HOOK_RET(ret, name, args...)  \
    do { \
        if (HAS_HOOK(name)) { \
            ret = cl_##name##_hook(args); \
        } else { \
            HOOK_PANIC(name); \
        } \
    } while (0)
*/

#define DEFINE_ENABLE_FUNC(name) \
    void enable_##name(void) {} \
    EXPORT_SYMBOL(enable_##name);

#define ENABLE_COMPONENT(name) \
    extern void enable_##name(); \
    enable_##name();

// Alias for ENABLE_COMPONENT
#define REQUIRE_COMPONENT(name) ENABLE_COMPONENT(name)

extern int cl_init();
extern int cl_top_init();

#endif /* _CL_HOOK_H_ */
