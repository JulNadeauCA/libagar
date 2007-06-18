/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/have_vsnprintf.h>
#include <config/_mk_have_sys_types_h.h>
#include <config/have_format_attribute.h>
#include <config/have_bounded_attribute.h>
#include <config/have_nonnull_attribute.h>
#else
#include <agar/config/have_vsnprintf.h>
#include <agar/config/_mk_have_sys_types_h.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#endif

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_VSNPRINTF
#include <stdio.h>
#else
# ifdef HAVE_BOUNDED_ATTRIBUTE
# define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
# else
# define BOUNDED_ATTRIBUTE(t, a, b)
# endif
# ifdef HAVE_FORMAT_ATTRIBUTE
# define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
# else
# define FORMAT_ATTRIBUTE(t, a, b)
# endif
# ifdef HAVE_NONNULL_ATTRIBUTE
# define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
# else
# define NONNULL_ATTRIBUTE(a)
# endif
int vsnprintf(char *, size_t, const char *, va_list)
    FORMAT_ATTRIBUTE(printf, 3, 0)
    NONNULL_ATTRIBUTE(3)
    BOUNDED_ATTRIBUTE(__string__, 1, 2);
#endif
