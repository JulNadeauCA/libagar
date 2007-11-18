/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/_mk_have_sys_types_h.h>
#include <config/have_bounded_attribute.h>
#else
#include <agar/config/_mk_have_sys_types_h.h>
#include <agar/config/have_bounded_attribute.h>
#endif

#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef BOUNDED_ATTRIBUTE
# ifdef HAVE_BOUNDED_ATTRIBUTE
#  define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
# else
#  define BOUNDED_ATTRIBUTE(t, a, b)
# endif
#endif

size_t AG_Strlcpy(char *, const char *, size_t)
    BOUNDED_ATTRIBUTE(__string__, 1, 3);

#ifdef _AGAR_INTERNAL
#define Strlcpy AG_Strlcpy
#endif
