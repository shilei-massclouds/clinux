// SPDX-License-Identifier: GPL-2.0-only

#ifndef _BOOTRD_H_
#define _BOOTRD_H_

#ifdef CL_TOOLS
#include <stdint.h>
typedef uint32_t u32;
#endif

const char BOOTRD_MAGIC[] = "CLBD";
const char PROFILE_MAGIC[] = "PROF";

/*
 * BootRD layout
 * -----------------
 * Header
 * modules
 * profiles
 * Selected profile
 * -----------------
 */
struct bootrd_header {
    u32     magic;              /* magic "CLBD" */
    u32     version;
    u32     total_size;         /* include header itself */
    u32     mod_offset;         /* offset of modules data area */
    u32     mod_num;
    u32     profile_offset;     /* offset of profiles data area */
    u32     profile_num;
    u32     current_profile;    /* point to current profile */
};

/*
 * Profile layout
 * -----------------
 * Header
 * Reference for sorted modules
 * -----------------
 */
struct profile_header {
    u32     magic;      /* magic "PROF" */
    u32     version;
    u32     total_size; /* include header itself */
    u32     mod_num;    /* number of modules in this profile */
};

#endif /* _BOOTRD_H_ */
