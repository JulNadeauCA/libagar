/*	$Csoft: vsnprintf.h,v 1.2 2003/10/09 22:39:29 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_vsnprintf.h>
#include <sys/types.h>

#ifdef HAVE_VSNPRINTF
#include <stdio.h>
#else
int	vsnprintf(char *, size_t, const char *, va_list)
	    FORMAT_ATTRIBUTE(printf, 3, 0)
	    NONNULL_ATTRIBUTE(3)
	    BOUNDED_ATTRIBUTE(__string__, 1, 2);
#endif
