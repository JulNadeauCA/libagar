/*	Public domain	*/

#ifdef _AGAR_INTERNAL
#include <config/_mk_have_limits_h.h>
#include <config/have_long_long.h>
#include <config/have_long_double.h>
#else
#include <agar/config/_mk_have_limits_h.h>
#include <agar/config/have_long_long.h>
#include <agar/config/have_long_double.h>
#endif

#ifndef _AGAR_CORE_LIMITS_H
#define _AGAR_CORE_LIMITS_H

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

#ifdef __FLT_MIN__
# define AG_FLT_MIN __FLT_MIN__
#else
# define AG_FLT_MIN 1.175494351e-38f
#endif
#ifdef __FLT_MAX__
# define AG_FLT_MAX __FLT_MAX__
#else
# define AG_FLT_MAX 3.402823466e+38f
#endif
#ifdef __DBL_MIN__
# define AG_DBL_MIN __DBL_MIN__
#else
# define AG_DBL_MIN 2.2250738585072014e-308
#endif
#ifdef __DBL_MAX__
# define AG_DBL_MAX __DBL_MAX__
#else
# define AG_DBL_MAX 1.7976931348623158e+308
#endif
#ifdef HAVE_LONG_DOUBLE
# ifdef __LDBL_MIN__
#  define AG_LDBL_MIN __LDBL_MIN__
# else
#  define AG_LDBL_MIN 3.36210314311209350626e-4932l
# endif
# ifdef __LDBL_MAX__
#  define AG_LDBL_MAX __LDBL_MAX__
# else
#  define AG_LDBL_MAX 1.18973149535723176502e+4932l
# endif
#endif /* HAVE_LONG_DOUBLE */

#endif /* _AGAR_CORE_LIMITS_H */
