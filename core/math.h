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

#ifdef M_PI
#define AG_PI M_PI
#else
#define	AG_PI 3.14159265358979323846
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN3
#define	MIN3(a,b,c) MIN((a),MIN((b),(c)))
#endif
#ifndef MAX3
#define	MAX3(a,b,c) MAX((a),MAX((b),(c)))
#endif

#endif /* _COMPAT_MATH_H */
