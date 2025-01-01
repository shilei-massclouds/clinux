/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _RISCV_IMAGE_H
#define _RISCV_IMAGE_H

#include <types.h>

/**
 * struct image_header - riscv kernel image header
 * @code0:      Executable code
 * @code1:      Executable code
 * @text_offset:Image load offset (little endian)
 * @image_size: Effective Image size (little endian)
 * @flags:      kernel flags (little endian)
 * @version:    version
 * @res1:       reserved
 * @res2:       reserved
 * @magic:      Magic number (RISC-V specific; deprecated)
 * @magic2:     Magic number 2 (to match the ARM64 'magic' field pos)
 * @res3:       reserved (will be used for PE COFF offset)
 *
 * The intention is for this header format to be shared between multiple
 * architectures to avoid a proliferation of image header formats.
 */

struct image_header {
    u32 code0;
    u32 code1;
    u64 text_offset;
    u64 image_size;
    u64 flags;
    u32 version;
    u32 res1;
    u64 res2;
    u64 magic;
    u32 magic2;
    u32 res3;
};

#endif  /* _RISCV_IMAGE_H */
