#ifndef _LINUX_HASH_H
#define _LINUX_HASH_H

#define hash_64 hash_64_generic

#define hash_long(val, bits) hash_64(val, bits)

#define GOLDEN_RATIO_32 0x61C88647
#define GOLDEN_RATIO_64 0x61C8864680B583EBull
#define GOLDEN_RATIO_PRIME GOLDEN_RATIO_64

static __always_inline u32 hash_64_generic(u64 val, unsigned int bits)
{
    /* 64x64-bit multiply is efficient on all 64-bit processors */
    return val * GOLDEN_RATIO_PRIME >> (64 - bits);
}

#define __hash_32 __hash_32_generic
static inline u32 __hash_32_generic(u32 val)
{
    return val * GOLDEN_RATIO_32;
}

#define hash_32 hash_32_generic
static inline u32 hash_32_generic(u32 val, unsigned int bits)
{
    /* High bits are more random, so use them. */
    return __hash_32(val) >> (32 - bits);
}

static inline u32 hash_ptr(const void *ptr, unsigned int bits)
{
    return hash_long((unsigned long)ptr, bits);
}

#endif /* _LINUX_HASH_H */
