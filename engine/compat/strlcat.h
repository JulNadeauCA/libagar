/*	$Csoft: strlcat.h,v 1.6 2004/01/23 06:12:26 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_strlcat.h>
#include <config/have_bounded_attribute.h>

#ifndef BOUNDED_ATTRIBUTE
# ifdef HAVE_BOUNDED_ATTRIBUTE
#  define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
# else
#  define BOUNDED_ATTRIBUTE(t, a, b)
# endif
#endif

#ifndef HAVE_STRLCAT
#include <sys/types.h>
size_t	strlcat(char *, const char *, size_t)
	    BOUNDED_ATTRIBUTE(__string__, 1, 3);
#else
#include <string.h>
#endif
