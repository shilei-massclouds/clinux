/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_CIRC_BUF_H
#define _LINUX_CIRC_BUF_H

struct circ_buf {
    char *buf;
    int head;
    int tail;
};

/* Return space available up to the end of the buffer.  */
#define CIRC_SPACE_TO_END(head,tail,size) \
    ({int end = (size) - 1 - (head); \
      int n = (end + (tail)) & ((size)-1); \
      n <= end ? n : end+1;})

#endif /* _LINUX_CIRC_BUF_H */
