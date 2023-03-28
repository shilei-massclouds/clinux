/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_ATTRIBUTES_H
#define __LINUX_COMPILER_ATTRIBUTES_H

#define __packed                __attribute__((__packed__))
#define __section(S)            __attribute__((__section__(#S)))
#define __cold                  __attribute__((__cold__))
#define __aligned(x)            __attribute__((__aligned__(x)))
#define __attribute_const__     __attribute__((__const__))
#define __always_inline         inline __attribute__((__always_inline__))

#define __force

/*
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-format-function-attribute
 * clang: https://clang.llvm.org/docs/AttributeReference.html#format
 */
#define __printf(a, b)  __attribute__((__format__(printf, a, b)))
#define __scanf(a, b)   __attribute__((__format__(scanf, a, b)))

#define likely(x)   (x)
#define unlikely(x) (x)

/*
 * __unqual_scalar_typeof(x) - Declare an unqualified scalar type, leaving
 *                 non-scalar types unchanged.
 */
/*
 * Prefer C11 _Generic for better compile-times and simpler code. Note: 'char'
 * is not type-compatible with 'signed char', and we define a separate case.
 */
#define __scalar_type_to_expr_cases(type)               \
        unsigned type:  (unsigned type)0,           \
        signed type:    (signed type)0

#define __unqual_scalar_typeof(x) typeof(               \
        _Generic((x),                       \
             char:  (char)0,                \
             __scalar_type_to_expr_cases(char),     \
             __scalar_type_to_expr_cases(short),        \
             __scalar_type_to_expr_cases(int),      \
             __scalar_type_to_expr_cases(long),     \
             __scalar_type_to_expr_cases(long long),    \
             default: (x)))

#endif /* __LINUX_COMPILER_ATTRIBUTES_H */
