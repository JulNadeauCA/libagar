/*	$Csoft: vasprintf.h,v 1.7 2003/10/09 22:39:29 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_vasprintf.h>
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>

#ifndef FORMAT_ATTRIBUTE
# ifdef HAVE_FORMAT_ATTRIBUTE
#  define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
# else
#  define FORMAT_ATTRIBUTE(t, a, b) /* nothing */
# endif
#endif

#ifndef NONNULL_ATTRIBUTE
# ifdef HAVE_NONNULL_ATTRIBUTE
#  define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
# else
#  define NONNULL_ATTRIBUTE(a) /* nothing */
# endif
#endif

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

