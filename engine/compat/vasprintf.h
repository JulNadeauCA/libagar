/*	$Csoft: vasprintf.h,v 1.3 2002/08/23 09:00:50 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifdef HAVE_VASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
# include <stdarg.h>
extern int vasprintf(char **, const char *, va_list);
#endif

