/*	Public domain	*/
/*
 * Definitions internal to our headers
 */

#ifdef SINGLE_PRECISION
# define _M_UNDEFINED_SINGLE_PRECISION
# undef SINGLE_PRECISION
#endif
#ifdef DOUBLE_PRECISION
# define _M_UNDEFINED_DOUBLE_PRECISION
# undef DOUBLE_PRECISION
#endif
#ifdef QUAD_PRECISION
# define _M_UNDEFINED_QUAD_PRECISION
# undef QUAD_PRECISION
#endif
#include <agar/config/single_precision.h>
#include <agar/config/quad_precision.h>
#include <agar/config/double_precision.h>

#undef _MK_HAVE_UNSIGNED_TYPEDEFS
#include <agar/config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
# define _MK_HAVE_UNSIGNED_TYPEDEFS
# define Uint unsigned int
# define Uchar unsigned char
# define Ulong unsigned long
#endif
#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# define _M_DEFINED_CDECLS
# if defined(__cplusplus)
#  define __BEGIN_DECLS extern "C" {
#  define __END_DECLS   }
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

/* Definitions internal to the math library */
#ifdef _M_INTERNAL
# undef MAX
# define MAX(h,i) ((h) > (i) ? (h) : (i))
# undef MIN
# define MIN(l,o) ((l) < (o) ? (l) : (o))
#endif /* _M_INTERNAL */
