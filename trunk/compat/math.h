/*	$Csoft: math.h,v 1.2 2004/03/25 05:36:32 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_math.h>

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
