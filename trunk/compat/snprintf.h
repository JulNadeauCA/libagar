/*	$Csoft: snprintf.h,v 1.2 2003/10/09 22:39:29 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_snprintf.h>

#include <sys/types.h>

#ifdef HAVE_SNPRINTF
#include <stdio.h>
#else
int	snprintf(char *, size_t, const char *, ...)
	    FORMAT_ATTRIBUTE(printf, 3, 4)
	    NONNULL_ATTRIBUTE(3)
	    BOUNDED_ATTRIBUTE(__string__, 1, 2);
#endif

