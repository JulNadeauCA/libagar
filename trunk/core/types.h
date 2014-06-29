/*	Public domain	*/

/*
 * Definitions of primitive integer types used throughout Agar. Also
 * defines AG_HAVE_64BIT and AG_HAVE_LONG_DOUBLE.
 */

#include <agar/config/have_cygwin.h>

#if defined(HAVE_CYGWIN)
# include <basetyps.h>
#else
# if !defined(_WIN32)
#  include <agar/config/_mk_have_sys_types_h.h>
#  ifdef _MK_HAVE_SYS_TYPES_H
#   undef _MK_HAVE_SYS_TYPES_H
#   include <sys/types.h>
#  endif
# endif /* !_WIN32 */
#endif /* HAVE_CYGWIN */

#ifndef _AGAR_HAVE_64BIT_H
# include <agar/config/have_64bit.h>
# define _AGAR_HAVE_64BIT_H_
# ifdef HAVE_64BIT
#  include <agar/config/have_int64_t.h>
#  include <agar/config/have___int64.h>
#  define AG_HAVE_64BIT
# endif
#endif

#ifndef _AGAR_HAVE_LONG_DOUBLE_H_
# include <agar/config/have_long_double.h>
# define _AGAR_HAVE_LONG_DOUBLE_H_
# ifdef HAVE_LONG_DOUBLE
#  define AG_HAVE_LONG_DOUBLE
# endif
#endif

#ifndef Uint
# define Uint unsigned int
# define _AGAR_CORE_DEFINED_UINT
#endif
#ifndef Uchar
# define Uchar unsigned char
# define _AGAR_CORE_DEFINED_UCHAR
#endif
#ifndef Ulong
# define Ulong unsigned long
# define _AGAR_CORE_DEFINED_ULONG
#endif

#if defined(_WIN32)
# ifndef Sint8
#  define Sint8 __int8
#  define _AGAR_CORE_DEFINED_SINT8
# endif
# ifndef Uint8
#  define Uint8 unsigned __int8
#  define _AGAR_CORE_DEFINED_UINT8
# endif
# ifndef Sint16
#  define Sint16 __int16
#  define _AGAR_CORE_DEFINED_SINT16
# endif
# ifndef Uint16
#  define Uint16 unsigned __int16
#  define _AGAR_CORE_DEFINED_UINT16
# endif
# ifndef Sint32
#  define Sint32 __int32
#  define _AGAR_CORE_DEFINED_SINT32
# endif
# ifndef Uint32
#  define Uint32 unsigned __int32
#  define _AGAR_CORE_DEFINED_UINT32
# endif
# ifndef Sint64
#  define Sint64 __int64
#  define _AGAR_CORE_DEFINED_SINT64
# endif
# ifndef Uint64
#  define Uint64 unsigned __int64
#  define _AGAR_CORE_DEFINED_UINT64
# endif
#else /* !_WIN32 */
# ifndef Sint8
#  define Sint8 int8_t
#  define _AGAR_CORE_DEFINED_SINT8
# endif
# ifndef Uint8
#  define Uint8 u_int8_t
#  define _AGAR_CORE_DEFINED_UINT8
# endif
# ifndef Sint16
#  define Sint16 int16_t
#  define _AGAR_CORE_DEFINED_SINT16
# endif
# ifndef Uint16
#  define Uint16 u_int16_t
#  define _AGAR_CORE_DEFINED_UINT16
# endif
# ifndef Sint32
#  define Sint32 int32_t
#  define _AGAR_CORE_DEFINED_SINT32
# endif
# ifndef Uint32
#  define Uint32 u_int32_t
#  define _AGAR_CORE_DEFINED_UINT32
# endif
# ifdef AG_HAVE_64BIT
#  ifndef Sint64
#   if defined(HAVE_INT64_T)
#    define Sint64 int64_t
#   elif defined(HAVE___INT64)
#    define Sint64 __int64
#   endif
#   define _AGAR_CORE_DEFINED_SINT64
#  endif
#  ifndef Uint64
#   if defined(HAVE_INT64_T)
#    define Uint64 u_int64_t
#   elif defined(HAVE___INT64)
#    define Uint64 __int64
#   endif
#   define _AGAR_CORE_DEFINED_UINT64
#  endif
# else /* !AG_HAVE_64BIT */
typedef struct ag_fake_int64 { Uint32 _pad1; Uint32 _pad2; };
#  ifndef Sint64
#   define Sint64 struct ag_fake_int64
#   define _AGAR_CORE_DEFINED_SINT64
#  endif
#  ifndef Uint64
#   define Uint64 struct ag_fake_int64
#   define _AGAR_CORE_DEFINED_UINT64
#  endif
# endif /* AG_HAVE_64BIT */
#endif /* !WIN32 */
