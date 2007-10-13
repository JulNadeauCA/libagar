/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/_mk_have_limits_h.h>
#include <config/have_long_double.h>
#include <config/have_long_long.h>
#else
#include <agar/config/_mk_have_limits_h.h>
#include <agar/config/have_long_double.h>
#include <agar/config/have_long_long.h>
#endif

#ifndef _COMPAT_LIMITS_H
#define _COMPAT_LIMITS_H

#ifdef _MK_HAVE_LIMITS_H
# include <limits.h>
# define AG_INT_MIN	INT_MIN
# define AG_INT_MAX	INT_MAX
# define AG_UINT_MAX	UINT_MAX
# define AG_LONG_MIN	LONG_MIN
# define AG_LONG_MAX	LONG_MAX
# define AG_ULONG_MAX	ULONG_MAX
#else
# define AG_INT_MIN	(-0x7fffffff-1)
# define AG_INT_MAX	0x7fffffff
# define AG_UINT_MAX	0xffffffffU
# ifdef __LP64__
#  define AG_ULONG_MAX	0xffffffffffffffffUL
#  define AG_LONG_MAX	0x7fffffffffffffffL	
#  define AG_LONG_MIN	(-0x7fffffffffffffffL-1)	
# else
#  define AG_ULONG_MAX	0xffffffffUL
#  define AG_LONG_MAX	0x7fffffffL
#  define AG_LONG_MIN	(-0x7fffffffL-1)
# endif
#endif
#ifdef HAVE_LONG_LONG
# define AG_ULLONG_MAX	0xffffffffffffffffULL	
# define AG_LLONG_MIN	(-0x7fffffffffffffffLL-1)	
# define AG_LLONG_MAX	0x7fffffffffffffffLL	
#endif
#ifdef _MK_HAVE_LIMITS_H_FP
# define AG_FLT_MIN	FLT_MIN
# define AG_FLT_MAX	FLT_MAX
# define AG_DBL_MIN	DBL_MIN
# define AG_DBL_MAX	DBL_MAX
#else
# define AG_FLT_MIN	1.175494351e-38F
# define AG_FLT_MAX	3.402823466e+38F
# define AG_DBL_MIN	2.2250738585072014e-308
# define AG_DBL_MAX	1.7976931348623158e+308
#endif

#endif /* _COMPAT_LIMITS_H */
