/*	$Csoft: math.h,v 1.5 2005/05/19 03:47:20 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_math.h>

#ifndef _COMPAT_MATH_H
#define _COMPAT_MATH_H

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

__BEGIN_DECLS
__inline__ int ftrunc(double);
__inline__ double ffrac(double);
__inline__ double finvfrac(double);
__inline__ int powof2(int);
__END_DECLS

#endif /* _COMPAT_MATH_H */
