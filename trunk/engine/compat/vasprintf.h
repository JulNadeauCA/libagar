/*	$Csoft: vasprintf.h,v 1.6 2003/06/18 00:46:59 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_vasprintf.h>

#ifdef HAVE_VASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
# include <stdarg.h>
int	vasprintf(char **, const char *, va_list)
	    FORMAT_ATTRIBUTE(printf, 2, 0)
	    NONNULL_ATTRIBUTE(2);
#endif

