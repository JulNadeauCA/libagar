/*	$Csoft: asprintf.h,v 1.6 2003/06/18 00:46:59 vedge Exp $	*/
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

