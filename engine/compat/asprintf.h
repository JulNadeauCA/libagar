/*	$Csoft: asprintf.h,v 1.5 2002/12/24 10:29:21 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_asprintf.h>

#ifdef HAVE_ASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
int	asprintf(char **, const char *, ...);
#endif

