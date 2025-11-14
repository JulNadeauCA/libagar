/*	Public domain	*/

#ifndef	_AGAR_CORE_LIMITS_H_
#define	_AGAR_CORE_LIMITS_H_

#include <agar/config/_mk_have_limits_h.h>
#include <agar/config/_mk_have_float_h.h>
#include <agar/config/have_long_long.h>

/*
 * String limits
 */
#if defined(MAXPATHLEN)
# define AG_PATHNAME_MAX MAXPATHLEN
#else
# if AG_MODEL == AG_SMALL
#  define AG_PATHNAME_MAX 64
# else
#  define AG_PATHNAME_MAX 1024
# endif
#endif

#if defined(FILENAME_MAX)
# define AG_FILENAME_MAX FILENAME_MAX
#else
# if AG_MODEL == AG_SMALL
#  define AG_FILENAME_MAX (16+1)
# elif AG_MODEL == AG_MEDIUM
#  define AG_FILENAME_MAX 128
# else
#  define AG_FILENAME_MAX 256
# endif
#endif

#if defined(_XBOX)
# define AG_ARG_MAX (MAX_LAUNCH_DATA_SIZE - 520)
#elif defined(ARG_MAX)
# define AG_ARG_MAX ARG_MAX
#else
# if AG_MODEL == AG_SMALL
#  define AG_ARG_MAX 256
# else
#  define AG_ARG_MAX 4096
# endif
#endif

#if defined(BUFSIZ)
# define AG_BUFFER_MIN BUFSIZ
# define AG_BUFFER_MAX BUFSIZ
#else
# if AG_MODEL == AG_SMALL
#  define AG_BUFFER_MIN 256
#  define AG_BUFFER_MAX 1024
# else
#  define AG_BUFFER_MIN 1024
#  define AG_BUFFER_MAX 8192
# endif
#endif

/*
 * Integer limits
 */
#ifdef _MK_HAVE_LIMITS_H

# include <limits.h>

# define AG_INT_MIN	INT_MIN
# define AG_INT_MAX	INT_MAX
# define AG_UINT_MAX	UINT_MAX
# define AG_LONG_MIN	LONG_MIN
# define AG_LONG_MAX	LONG_MAX
# define AG_ULONG_MAX	ULONG_MAX

#else /* !HAVE_LIMITS_H */

# if AG_MODEL == AG_SMALL

#  define AG_INT_MIN   (-0x7ffe)
#  define AG_INT_MAX     0x7fff
#  define AG_UINT_MAX    0xffff
#  define AG_LONG_MIN  (-0x7ffffffe)
#  define AG_LONG_MAX    0x7fffffff
#  define AG_ULONG_MAX   0xffffffffU

# else /* MEDIUM or LARGE */

#  define AG_INT_MIN   (-0x7ffffffe)
#  define AG_INT_MAX     0x7fffffff
#  define AG_UINT_MAX    0xffffffffU
#  ifdef __LP64__
#   define AG_ULONG_MAX  0xffffffffffffffffUL
#   define AG_LONG_MAX   0x7fffffffffffffffL	
#   define AG_LONG_MIN (-0x7ffffffffffffffeL)	
#  else
#   define AG_ULONG_MAX  0xffffffffUL
#   define AG_LONG_MAX	 0x7fffffffL
#   define AG_LONG_MIN (-0x7ffffffeL)
#  endif
#  ifdef HAVE_LONG_LONG
#   define AG_ULLONG_MAX  0xffffffffffffffffULL	
#   define AG_LLONG_MIN (-0x7ffffffffffffffeLL)	
#   define AG_LLONG_MAX   0x7fffffffffffffffLL	
#  endif
# endif /* MEDIUM or LARGE */

#endif /* !HAVE_LIMITS_H */

/*
 * Floating-point number limits
 */
#ifdef AG_HAVE_FLOAT
# ifdef __FLT_MIN__
#  define AG_FLT_MIN __FLT_MIN__
# else
#  define AG_FLT_MIN 1.175494351e-38f
# endif
# ifdef __FLT_MAX__
#  define AG_FLT_MAX __FLT_MAX__
# else
#  define AG_FLT_MAX 3.402823466e+38f
# endif
# ifdef __DBL_MIN__
#  define AG_DBL_MIN __DBL_MIN__
# else
#  define AG_DBL_MIN 2.2250738585072014e-308
# endif
# ifdef __DBL_MAX__
#  define AG_DBL_MAX __DBL_MAX__
# else
#  define AG_DBL_MAX 1.7976931348623158e+308
# endif

# ifdef _MK_HAVE_FLOAT_H
#  include <float.h>
#  define AG_FLT_EPSILON FLT_EPSILON
#  define AG_DBL_EPSILON DBL_EPSILON
# else /* !HAVE_FLOAT_H */
#  define AG_FLT_EPSILON 1.192092896e-7f
#  define AG_DBL_EPSILON 2.2204460492503131e-16
# endif /* !HAVE_FLOAT_H */

#endif /* AG_HAVE_FLOAT */

#endif /* _AGAR_CORE_LIMITS_H_ */
