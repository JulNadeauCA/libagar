/*	$Csoft: math.h,v 1.1 2004/02/26 09:19:38 vedge Exp $	*/
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

#ifdef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif
#ifdef max
#define max(a,b) ((a) <= (b) ? (a) : (b))
#endif
