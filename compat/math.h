/*	$Csoft: math.h,v 1.3 2004/03/25 05:37:32 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_math.h>

#ifndef _COMPAT_MATH_H
#define _COMPAT_MATH_H

#ifdef sgi
/*
 * Irix's <math.h> defines an 'enum version' which conflicts with the
 * 'struct version' of <engine/version.h>.
 */
#undef _SGIAPI
#endif

#ifdef HAVE_MATH
#include <math.h>
#endif

#ifndef FLT_MAX
#define FLT_MAX		3.40282347E+38F		/* (1-b**(-p))*b**emax */
#endif
#ifndef DBL_MAX
#define DBL_MAX		1.7976931348623157E+308
#endif

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) <= (b) ? (a) : (b))
#endif

typedef Uint32 fix6;
typedef Uint32 fix8;
typedef Uint32 fix16;
typedef Uint32 fix24;
typedef Uint32 fix30;

#define FIX6_MAX_INT	(1<<25)
#define FIX8_MAX_INT	(1<<24)
#define FIX10_MAX_INT	(1<<22)
#define FIX16_MAX_INT	(1<<16)
#define FIX30_MAX_INT	(1<<2)

#define FIX6_MAX_FRAC	(1<<6)
#define FIX8_MAX_FRAC	(1<<8)
#define FIX10_MAX_FRAC	(1<<10)
#define FIX16_MAX_FRAC	(1<<16)
#define FIX30_MAX_FRAC	(1<<30)

#define inttofix6(i) ((i) << 6)
#define inttofix8(i) ((i) << 8)
#define inttofix10(i) ((i) << 10)
#define inttofix16(i) ((i) << 16)
#define inttofix30(i) ((i) << 30)

#define fix6toint(f) ((f) >> 6)
#define fix8toint(f) ((f) >> 8)
#define fix10toint(f) ((f) >> 10)
#define fix16toint(f) ((f) >> 16)
#define fix30toint(f) ((f) >> 30)

#define fix6frac(f) ((f) << 26)
#define fix8frac(f) ((f) << 24)
#define fix10frac(f) ((f) << 22)
#define fix16frac(f) ((f) << 16)
#define fix30frac(f) ((f) << 2)

#define fix6invfrac(f) (1-((f) << 26))
#define fix8invfrac(f) (1-((f) << 24))
#define fix10invfrac(f) (1-((f) << 22))
#define fix16invfrac(f) (1-((f) << 16))
#define fix30invfrac(f) (1-((f) << 2))

#define fptofix6(F) ((fix6)((F) * 64))
#define fptofix8(F) ((fix8)((F) * 256))
#define fptofix10(F) ((fix10)((F) * 1024))
#define fptofix16(F) ((fix16)((F) * 65536))
#define fptofix30(F) ((fix30)((F) * 2147483648))

#define fix6tofp(f) (((float)(f)) / 64)
#define fix8tofp(f) (((float)(f)) / 256)
#define fix10tofp(f) (((float)(f)) / 1024)
#define fix16tofp(f) (((float)(f)) / 65536)
#define fix30tofp(f) (((float)(f)) / 2147483648)

#define fix6mul(f1, f2) (((f1)*(f2)) >> 6)
#define fix8mul(f1, f2) (((f1)*(f2)) >> 8)
#define fix10mul(f1, f2) (((f1)*(f2)) >> 10)
#define fix16mul(f1, f2) (((f1)*(f2)) >> 16)
#define fix30mul(f1, f2) (((f1)*(f2)) >> 30)

#define fix6div(f1, f2) (((f1) << 6) / (f2))
#define fix8div(f1, f2) (((f1) << 8) / (f2))
#define fix10div(f1, f2) (((f1) << 10) / (f2))
#define fix16div(f1, f2) (((f1) << 16) / (f2))
#define fix30div(f1, f2) (((f1) << 30) / (f2))

__BEGIN_DECLS
__inline__ Uint32	fix6trunc(fix6);
fix30			fix30sqrt(fix30);
__END_DECLS

#endif /* _COMPAT_MATH_H */
