/*	$Csoft: strlcpy.h,v 1.5 2003/10/09 22:39:29 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcpy.h>
#include <config/have_bounded_attribute.h>

#ifndef BOUNDED_ATTRIBUTE
# ifdef HAVE_BOUNDED_ATTRIBUTE
#  define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
# else
#  define BOUNDED_ATTRIBUTE(t, a, b)
# endif
#endif

#ifndef HAVE_STRLCPY
#include <sys/types.h>
size_t	strlcpy(char *, const char *, size_t)
	    BOUNDED_ATTRIBUTE(__string__, 1, 3);
#else
#include <string.h>
#endif

