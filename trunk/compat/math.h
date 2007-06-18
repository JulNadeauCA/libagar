/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_math.h>
#else
#include <agar/config/have_math.h>
#endif

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

#endif /* _COMPAT_MATH_H */
