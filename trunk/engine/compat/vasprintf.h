/*	$Csoft: vasprintf.h,v 1.5 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_vasprintf.h>

#ifdef HAVE_VASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
# include <stdarg.h>
int	vasprintf(char **, const char *, va_list);
#endif

