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

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) <= (b) ? (a) : (b))
#endif
#endif /* _COMPAT_MATH_H */
