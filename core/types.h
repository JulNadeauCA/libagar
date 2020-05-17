/*	Public domain	*/

/*
 * Agar memory model
 */
#define AG_SMALL  16   /*  8-/16-bit CPU with KBs of RAM, 12-bit color */
#define AG_MEDIUM 32   /* 32-/64-bit CPU with MBs of RAM, 24-bit color */
#define AG_LARGE  64   /* 32-/64-bit CPU with GBs of RAM, 48-bit color */

#include <agar/config/ag_model.h>

#include <agar/config/have_cygwin.h>
#if defined(HAVE_CYGWIN)
# include <basetyps.h>
#else
# if !defined(_WIN32)
#  include <agar/config/_mk_have_sys_types_h.h>
#  include <agar/config/_mk_have_stdint_h.h>
#  if defined(_MK_HAVE_SYS_TYPES_H)
#   undef _MK_HAVE_SYS_TYPES_H
#   include <sys/types.h>
#  elif defined(_MK_HAVE_STDINT_H)
#   include <stdint.h>
#  endif
# endif /* !_WIN32 */
#endif /* HAVE_CYGWIN */

#include <agar/config/have_64bit.h>
#ifdef HAVE_64BIT
# include <agar/config/have_int64_t.h>
# include <agar/config/have___int64.h>
# define AG_HAVE_64BIT "yes"
#endif
#ifndef _AGAR_INTERNAL
# undef HAVE_64BIT
#endif

/*
 * Floating Point Types
 */
#include <agar/config/have_float.h>
#ifdef HAVE_FLOAT
# define AG_HAVE_FLOAT "yes"
#endif
#ifndef _AGAR_INTERNAL
# undef HAVE_FLOAT
#endif

/*
 * C Integer Types
 */
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

/*
 * Fixed-Size Integer Types
 */
#if (AG_MODEL == AG_SMALL)
# ifndef Sint8
#  define Sint8 int8_t
#  define _AGAR_CORE_DEFINED_SINT8
# endif
# ifndef Uint8
#  define Uint8 uint8_t
#  define _AGAR_CORE_DEFINED_UINT8
# endif
# ifndef Sint16
#  define Sint16 int16_t
#  define _AGAR_CORE_DEFINED_SINT16
# endif
# ifndef Uint16
#  define Uint16 uint16_t
#  define _AGAR_CORE_DEFINED_UINT16
# endif
# ifndef Sint32
#  define Sint32 int32_t
#  define _AGAR_CORE_DEFINED_SINT32
# endif
# ifndef Uint32
#  define Uint32 uint32_t
#  define _AGAR_CORE_DEFINED_UINT32
# endif

#else /* MEDIUM or LARGE */

# if defined(_WIN32)
#  ifndef Sint8
#   define Sint8 __int8
#   define _AGAR_CORE_DEFINED_SINT8
#  endif
#  ifndef Uint8
#   define Uint8 unsigned __int8
#   define _AGAR_CORE_DEFINED_UINT8
#  endif
#  ifndef Sint16
#   define Sint16 __int16
#   define _AGAR_CORE_DEFINED_SINT16
#  endif
#  ifndef Uint16
#   define Uint16 unsigned __int16
#   define _AGAR_CORE_DEFINED_UINT16
#  endif
#  ifndef Sint32
#   define Sint32 __int32
#   define _AGAR_CORE_DEFINED_SINT32
#  endif
#  ifndef Uint32
#   define Uint32 unsigned __int32
#   define _AGAR_CORE_DEFINED_UINT32
#  endif
#  ifndef Sint64
#   define Sint64 __int64
#   define _AGAR_CORE_DEFINED_SINT64
#  endif
#  ifndef Uint64
#   define Uint64 unsigned __int64
#   define _AGAR_CORE_DEFINED_UINT64
#  endif

# else /* !_WIN32 */

#  ifndef Sint8
#   define Sint8 int8_t
#   define _AGAR_CORE_DEFINED_SINT8
#  endif
#  ifndef Uint8
#   define Uint8 u_int8_t
#   define _AGAR_CORE_DEFINED_UINT8
#  endif
#  ifndef Sint16
#   define Sint16 int16_t
#   define _AGAR_CORE_DEFINED_SINT16
#  endif
#  ifndef Uint16
#   define Uint16 u_int16_t
#   define _AGAR_CORE_DEFINED_UINT16
#  endif
#  ifndef Sint32
#   define Sint32 int32_t
#   define _AGAR_CORE_DEFINED_SINT32
#  endif
#  ifndef Uint32
#   define Uint32 u_int32_t
#   define _AGAR_CORE_DEFINED_UINT32
#  endif
#  ifdef AG_HAVE_64BIT
#   ifndef Sint64
#    if defined(HAVE_INT64_T)
#     define Sint64 int64_t
#     define _AGAR_CORE_DEFINED_SINT64
#    elif defined(HAVE___INT64)
#     define Sint64 __int64
#     define _AGAR_CORE_DEFINED_SINT64
#    else
#     error "No Sint64 type"
#    endif
#   endif
#   ifndef Uint64
#    if defined(HAVE_INT64_T)
#     define Uint64 u_int64_t
#     define _AGAR_CORE_DEFINED_UINT64
#    elif defined(HAVE___INT64)
#     define Uint64 __int64
#     define _AGAR_CORE_DEFINED_UINT64
#    else
#     error "No Uint64 type"
#    endif
#   endif
#  else
#   warning "64-bit types not supported"
#  endif /* AG_HAVE_64BIT */

# endif /* !WIN32 */

#endif /* MEDIUM or LARGE */

/*
 * Native Character Type
 */
#include <agar/config/ag_unicode.h>
#ifdef AG_UNICODE
# define AG_Char Uint32
# define AG_CHAR_MAX 0x7fffffff
#else
# define AG_Char Uint8
# define AG_CHAR_MAX 0x7f
#endif

/*
 * Size and Offset Types
 */
#if AG_MODEL == AG_SMALL
# define AG_Size     Uint16
# define AG_Offset   Sint16
# define AG_SIZE_MAX 0xffff
# define AG_OFFS_MAX 0x7fff
#elif AG_MODEL == AG_MEDIUM
# define AG_Size     Uint32
# define AG_Offset   Sint32
# define AG_SIZE_MAX 0xffffffff
# define AG_OFFS_MAX 0x7fffffff
#elif AG_MODEL == AG_LARGE
# define AG_Size     Uint64
# define AG_Offset   Sint64
# define AG_SIZE_MAX 0xffffffffffffffff
# define AG_OFFS_MAX 0x7fffffffffffffff
#else
# error "Invalid AG_MODEL"
#endif
