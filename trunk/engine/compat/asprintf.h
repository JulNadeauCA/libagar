/*	$Csoft: vasprintf.h,v 1.4 2002/11/07 19:00:14 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mcconfig.h>

#ifdef HAVE_ASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
extern int asprintf(char **, const char *, ...);
#endif

