/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_asprintf.h>
#else
#include <agar/config/have_asprintf.h>
#endif

#ifdef HAVE_ASPRINTF
# ifdef __linux__
#  define _GNU_SOURCE
# endif
# include <stdio.h>
#else
int	asprintf(char **, const char *, ...);
#endif
