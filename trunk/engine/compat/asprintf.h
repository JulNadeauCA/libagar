/*	$Csoft: asprintf.h,v 1.4 2002/11/22 23:00:53 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_asprintf.h>

#ifdef HAVE_ASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
extern int asprintf(char **, const char *, ...);
#endif

