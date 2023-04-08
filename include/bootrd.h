// SPDX-License-Identifier: GPL-2.0-only

#ifndef _BOOTRD_H_
#define _BOOTRD_H_

#ifdef CL_TOOLS
#include <stdint.h>
typedef uint16_t u16;
typedef uint32_t u32;
#endif

const char BOOTRD_MAGIC[] = "CLBD";
const char PROFILE_MAGIC[] = "PROF";
const char PAYLOAD_MAGIC[] = "LOAD";

/*
 * BootRD layout
 * -----------------
 * Header
 * modules
 * profiles
 * Selected profile
 * payloads
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
    u32     payload_offset;     /* offset of payloads data area */
    u32     payload_num;
};

/*
 * Profile layout
 * -----------------
 * Header
 *
 * sorted modules offsets
 * -----------------
 */
struct profile_header {
    u32     magic;      /* magic "PROF" */
    u32     version;
    u32     total_size; /* include header itself */
    u32     mod_num;    /* number of modules in this profile */
    char    sname[16];  /* short name */
};

#define PAYLOAD_PAGE_ALIGN 0x0001 /* require page-aligned */

/*
 * Payload layout
 * -----------------
 * Header
 *
 * Payload -- data
 *         |
 *         -- name
 * [Payload]
 * -----------------
 */
struct payload_header {
    u32 magic;          /* magic "LOAD" */
    u16 version;
    u16 flags;
    u32 total_size;     /* include header itself */
    u32 name_offset;    /* offset for payload name */
};

#endif /* _BOOTRD_H_ */
