/*	$Csoft$	*/
/*	Public domain	*/

#ifdef sgi
/*
 * Irix's <math.h> defines an 'enum version' which conflicts with the
 * 'struct version' of <engine/version.h>.
 */
#undef _SGIAPI
#endif

#include <math.h>

#ifndef FLT_MAX
#define FLT_MAX		3.40282347E+38F		/* (1-b**(-p))*b**emax */
#endif
#ifndef DBL_MAX
#define DBL_MAX		1.7976931348623157E+308
#endif

