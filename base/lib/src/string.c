// SPDX-License-Identifier: GPL-2.0

#include <types.h>
#include <bitops.h>
#include <string.h>
#include <export.h>
#include <uaccess.h>
#include <word-at-a-time.h>

const unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,                /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,         /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,                /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,                /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,                /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,                /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,                /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,                /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,      /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,                /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,                /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,                /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,      /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,                /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,                /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,                /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,    /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,    /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,    /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,    /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,    /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};   /* 240-255 */

EXPORT_SYMBOL(_ctype);

/**
 * memchr - Find a character in an area of memory.
 * @s: The memory area
 * @c: The byte to search for
 * @n: The size of the area.
 *
 * returns the address of the first occurrence of @c, or %NULL
 * if @c is not found
 */
void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = s;
    while (n-- != 0) {
        if ((unsigned char)c == *p++) {
            return (void *)(p - 1);
        }
    }
    return NULL;
}
EXPORT_SYMBOL(memchr);

/**
 * memmove - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * Unlike memcpy(), memmove() copes with overlapping areas.
 */
void *memmove(void *dest, const void *src, size_t count)
{
    char *tmp;
    const char *s;

    if (dest <= src) {
        tmp = dest;
        s = src;
        while (count--)
            *tmp++ = *s++;
    } else {
        tmp = dest;
        tmp += count;
        s = src;
        s += count;
        while (count--)
            *--tmp = *--s;
    }
    return dest;
}
EXPORT_SYMBOL(memmove);

/**
 * strlen - Find the length of a string
 * @s: The string to be sized
 */
size_t strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}
EXPORT_SYMBOL(strlen);

/**
 * strcspn - Calculate the length of the initial substring of @s which does not contain letters in @reject
 * @s: The string to be searched
 * @reject: The string to avoid
 */
size_t strcspn(const char *s, const char *reject)
{
    const char *p;
    const char *r;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p) {
        for (r = reject; *r != '\0'; ++r) {
            if (*p == *r)
                return count;
        }
        ++count;
    }
    return count;
}
EXPORT_SYMBOL(strcspn);

/**
 * strrchr - Find the last occurrence of a character in a string
 * @s: The string to be searched
 * @c: The character to search for
 */
char *strrchr(const char *s, int c)
{
    const char *last = NULL;
    do {
        if (*s == (char)c)
            last = s;
    } while (*s++);
    return (char *)last;
}
EXPORT_SYMBOL(strrchr);

/**
 * strchr - Find the first occurrence of a character in a string
 * @s: The string to be searched
 * @c: The character to search for
 *
 * Note that the %NUL-terminator is considered part of the string, and can
 * be searched for.
 */
char *
strchr(const char *s, int c)
{
    for (; *s != (char)c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *)s;
}
EXPORT_SYMBOL(strchr);

/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int
strncmp(const char *cs, const char *ct, size_t count)
{
    unsigned char c1, c2;

    while (count) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
        count--;
    }
    return 0;
}
EXPORT_SYMBOL(strncmp);

/**
 * strchrnul - Find and return a character in a string, or end of string
 * @s: The string to be searched
 * @c: The character to search for
 *
 * Returns pointer to first occurrence of 'c' in s. If c is not found, then
 * return a pointer to the null byte at the end of s.
 */
char *
strchrnul(const char *s, int c)
{
    while (*s && *s != (char)c)
        s++;
    return (char *)s;
}
EXPORT_SYMBOL(strchrnul);

/**
 * strnlen - Find the length of a length-limited string
 * @s: The string to be sized
 * @count: The maximum number of bytes to search
 */
size_t
strnlen(const char *s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}
EXPORT_SYMBOL(strnlen);

int
strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;

    do {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
    } while (c1 == c2 && c1 != 0);
    return c1 - c2;
}
EXPORT_SYMBOL(strcasecmp);

/**
 * strreplace - Replace all occurrences of character in string.
 * @s: The string to operate on.
 * @old: The character being replaced.
 * @new: The character @old is replaced with.
 *
 * Returns pointer to the nul byte at the end of @s.
 */
char *strreplace(char *s, char old, char new)
{
    for (; *s; ++s)
        if (*s == old)
            *s = new;
    return s;
}
EXPORT_SYMBOL(strreplace);

size_t
strlcpy(char *dest, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return ret;
}
EXPORT_SYMBOL(strlcpy);

/**
 * skip_spaces - Removes leading whitespace from @str.
 * @str: The string to be stripped.
 *
 * Returns a pointer to the first non-whitespace character in @str.
 */
char *
skip_spaces(const char *str)
{
    while (isspace(*str))
        ++str;
    return (char *)str;
}
EXPORT_SYMBOL(skip_spaces);

/**
 * strcpy - Copy a %NUL terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 */
char *
strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}
EXPORT_SYMBOL(strcpy);

/*
 * Do a strncpy, return length of string without final '\0'.
 * 'count' is the user-supplied count (return 'count' if we
 * hit it), 'max' is the address space maximum (and we return
 * -EFAULT if we hit it).
 */
static inline long
do_strncpy_from_user(char *dst, const char *src,
                     unsigned long count, unsigned long max)
{
    unsigned long res = 0;
    const struct word_at_a_time constants = WORD_AT_A_TIME_CONSTANTS;

    while (max >= sizeof(unsigned long)) {
        unsigned long c, data;

        /* Fall back to byte-at-a-time if we get a page fault */
        unsafe_get_user(c, (unsigned long *)(src+res));

        *(unsigned long *)(dst+res) = c;
        if (has_zero(c, &data, &constants)) {
            data = prep_zero_mask(c, data, &constants);
            data = create_zero_mask(data);
            return res + find_zero(data);
        }
        res += sizeof(unsigned long);
        max -= sizeof(unsigned long);
    }

    panic("%s: !", __func__);
}

long strncpy_from_user(char *dst, const char *src, long count)
{
    unsigned long max_addr;
    unsigned long src_addr;

    if (unlikely(count <= 0))
        return 0;

    max_addr = user_addr_max();
    src_addr = (unsigned long)src;
    if (likely(src_addr < max_addr)) {
        long retval;
        unsigned long max = max_addr - src_addr;

        /*
         * Truncate 'max' to the user-specified limit, so that
         * we only have one limit we need to check in the loop
         */
        if (max > count)
            max = count;

        if (user_read_access_begin(src, max)) {
            retval = do_strncpy_from_user(dst, src, count, max);
            user_read_access_end();
            return retval;
        }
    }
    return -EFAULT;
}
EXPORT_SYMBOL(strncpy_from_user);

static inline long
do_strnlen_user(const char *src, unsigned long count, unsigned long max)
{
    unsigned long c;
    unsigned long align;
    unsigned long res = 0;
    const struct word_at_a_time constants = WORD_AT_A_TIME_CONSTANTS;

    /*
     * Do everything aligned. But that means that we
     * need to also expand the maximum..
     */
    align = (sizeof(unsigned long) - 1) & (unsigned long)src;
    src -= align;
    max += align;

    unsafe_get_user(c, (unsigned long *)src);
    c |= aligned_byte_mask(align);

    for (;;) {
        unsigned long data;
        if (has_zero(c, &data, &constants)) {
            data = prep_zero_mask(c, data, &constants);
            data = create_zero_mask(data);
            return res + find_zero(data) + 1 - align;
        }
        res += sizeof(unsigned long);
        /* We already handled 'unsigned long' bytes. Did we do it all ? */
        if (unlikely(max <= sizeof(unsigned long)))
            break;
        max -= sizeof(unsigned long);
        unsafe_get_user(c, (unsigned long *)(src+res));
    }
    res -= align;

    /*
     * Uhhuh. We hit 'max'. But was that the user-specified maximum
     * too? If so, return the marker for "too long".
     */
    if (res >= count)
        return count+1;

    /*
     * Nope: we hit the address space limit, and we still had more
     * characters the caller would have wanted. That's 0.
     */
    return 0;
}

long strnlen_user(const char *str, long count)
{
    unsigned long max_addr, src_addr;

    if (unlikely(count <= 0))
        return 0;

    max_addr = user_addr_max();
    src_addr = (unsigned long)untagged_addr(str);
    if (likely(src_addr < max_addr)) {
        long retval;
        unsigned long max = max_addr - src_addr;

        /*
         * Truncate 'max' to the user-specified limit, so that
         * we only have one limit we need to check in the loop
         */
        if (max > count)
            max = count;

        if (user_read_access_begin(str, max)) {
            retval = do_strnlen_user(str, count, max);
            user_read_access_end();
            return retval;
        }
    }
    return 0;
}
EXPORT_SYMBOL(strnlen_user);

/**
 * strncpy - Copy a length-limited, C-string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @count: The maximum number of bytes to copy
 *
 * The result is not %NUL-terminated if the source exceeds
 * @count bytes.
 *
 * In the case where the length of @src is less than  that  of
 * count, the remainder of @dest will be padded with %NUL.
 *
 */
char *strncpy(char *dest, const char *src, size_t count)
{
    char *tmp = dest;

    while (count) {
        if ((*tmp = *src) != 0)
            src++;
        tmp++;
        count--;
    }
    return dest;
}
EXPORT_SYMBOL(strncpy);
