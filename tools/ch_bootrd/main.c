// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "../../booter/src/bootrd.h"

static void
get_profile_header(FILE *fp, u32 offset, struct profile_header *ph)
{
    fseek(fp, offset, SEEK_SET);

    memset(ph, 0, sizeof(*ph));
    if (fread(ph, sizeof(*ph), 1, fp) != 1) {
        printf("cannot read profile err[%d]\n", errno);
        exit(-1);
    }

    if (memcmp(&(ph->magic), &PROFILE_MAGIC, sizeof(ph->magic))) {
        printf("profile: bad magic\n");
        exit(-1);
    }
    if (ph->version != 1) {
        printf("profile: bad version\n");
        exit(-1);
    }
}

static void
overwrite_current_profile_index(FILE *fp, u32 offset,
                                struct bootrd_header *bh)
{
    if (bh->current_profile == offset)
        return;

    bh->current_profile = offset;

    /* overwrite bootrd header */
    fseek(fp, 0, SEEK_SET);
    if (fwrite(bh, sizeof(*bh), 1, fp) != 1) {
        printf("%s: cannot overwrite bootrd header!\n", __func__);
        exit(-1);
    }
    //printf("%s: current offset(%x)\n", __func__, offset);
}

int
main(int argc, char *argv[])
{
    int index = -1;

    if (argc != 2 && argc != 4) {
        printf("usage: %s BOOTRD_FILE [-s profile_index]\n", argv[0]);
        printf("View or change(with -s) profiles of bootrdusage\n");
        exit(-1);
    }

    if (argc == 4) {
        if (strncmp(argv[2], "-s", 2) != 0) {
            printf("bad argument '%s'\n", argv[2]);
            exit(-1);
        }

        index = atoi(argv[3]);
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
    if (index >= (int)bh.profile_num) {
        printf("bad profile index(%d), it should be in [0, %u)\n",
               index, bh.profile_num);
        exit(-1);
    }
    printf("Bootrd profiles: offset(%x) curr[%x]:\n",
           bh.profile_offset, bh.current_profile);

    u32 offset = bh.profile_offset;

    int i;
    for (i = 0; i < bh.profile_num; i++) {
        struct profile_header ph;
        get_profile_header(fp, offset, &ph);

        char *prefix = "     ";
        if (index < 0) {
            if (offset == bh.current_profile)
                prefix = "  -> ";
        } else if (index == i) {
            overwrite_current_profile_index(fp, offset, &bh);
            prefix = "  -> ";
        }

        printf("%s[%d]: %s[%x]\n", prefix, i, ph.sname, offset);
        offset += ph.total_size;
    }

    fclose(fp);
    return 0;
}
