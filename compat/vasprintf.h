/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>
#else
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#endif

#ifndef FORMAT_ATTRIBUTE
# ifdef HAVE_FORMAT_ATTRIBUTE
#  define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
# else
#  define FORMAT_ATTRIBUTE(t, a, b)
# endif
#endif
#ifndef NONNULL_ATTRIBUTE
# ifdef HAVE_NONNULL_ATTRIBUTE
#  define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
# else
#  define NONNULL_ATTRIBUTE(a)
# endif
#endif

#include <stdarg.h>

int AG_Vasprintf(char **, const char *, va_list)
    FORMAT_ATTRIBUTE(printf, 2, 0)
    NONNULL_ATTRIBUTE(2);
