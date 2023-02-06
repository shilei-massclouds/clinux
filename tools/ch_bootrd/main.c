// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//#include "../../include/config.h"
#include "../../include/bootrd.h"

static u32
next_profile(FILE *fp, u32 offset)
{
    fseek(fp, offset, SEEK_SET);

    struct profile_header ph;
    memset(&ph, 0, sizeof(ph));
    if (fread(&ph, sizeof(ph), 1, fp) != 1) {
        printf("cannot read profile err[%d]\n", errno);
        exit(-1);
    }

    if (memcmp(&ph.magic, &PROFILE_MAGIC, sizeof(ph.magic))) {
        printf("profile: bad magic\n");
        exit(-1);
    }
    if (ph.version != 1) {
        printf("profile: bad version\n");
        exit(-1);
    }

    printf("profile: offset(%x) total_size(%x)\n",
           offset, ph.total_size);
    return (offset + ph.total_size);
}

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s [bootrd filename]\n", argv[0]);
        exit(-1);
    }

    printf("Bootrd filename: %s\n\n", argv[1]);
    FILE *fp = fopen(argv[1], "r+");
    if (fp == NULL) {
        printf("cannot open file '%s' err[%d]\n", argv[1], errno);
        exit(-1);
    }

    struct bootrd_header bh;
    memset(&bh, 0, sizeof(bh));
    if (fread(&bh, sizeof(bh), 1, fp) != 1) {
        printf("cannot read file '%s' err[%d]\n", argv[1], errno);
        exit(-1);
    }

    if (memcmp(&(bh.magic), &BOOTRD_MAGIC, sizeof(bh.magic))) {
        printf("bootrd: bad magic\n");
        exit(-1);
    }
    if (bh.version != 1) {
        printf("bootrd: bad version\n");
        exit(-1);
    }

    printf("Bootrd profiles: total[%d]:\n", bh.profile_num);
    printf("Bootrd profiles: offset(%x) curr[%x]:\n",
           bh.profile_offset, bh.current_profile);

    u32 offset = bh.profile_offset;

    int i;
    for (i = 0; i < bh.profile_num; i++) {
        char *prefix = (offset == bh.current_profile) ? "-> " : "   ";
        printf("%s[%d]: offset[%x]:\n", prefix, i, offset);
        offset = next_profile(fp, offset);
    }

    fclose(fp);
    return 0;
}
