/*	$Csoft: vasprintf.h,v 1.4 2002/11/07 19:00:14 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_vasprintf.h>

#ifdef HAVE_VASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
# include <stdarg.h>
extern int vasprintf(char **, const char *, va_list);
#endif

