/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

#ifndef _CONST_H
#define _CONST_H

#ifdef __ASSEMBLY__
#define _AC(X,Y)	X
#else
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#endif

#ifdef __ASSEMBLY__
#define __ASM_STR(x)    x
#else
#define __ASM_STR(x)    #x
#endif

#endif /* _CONST_H */
