/*	Public domain	*/

#ifdef _AGAR_BEGIN_H_
#error Nested inclusion of <agar/begin.h>
#endif
#define _AGAR_BEGIN_H_

/*
 * Expand "DECLSPEC" to any compiler-specific keywords, as required for proper
 * visibility of symbols in shared libraries.
 * See: http://gcc.gnu.org/wiki/Visibility
 */
#ifndef DECLSPEC
# if defined(__BEOS__)
#  if defined(__GNUC__)
#   define DECLSPEC	__declspec(dllexport)
#  else
#   define DECLSPEC	__declspec(export)
#  endif
# elif defined(__WIN32__)
#  ifdef __BORLANDC__
#   ifdef _AGAR_INTERNAL
#    define DECLSPEC 
#   else
#    define DECLSPEC	__declspec(dllimport)
#   endif
#  else
#   define DECLSPEC	__declspec(dllexport)
#  endif
# elif defined(__OS2__)
#  ifdef __WATCOMC__
#   ifdef _AGAR_INTERNAL
#    define DECLSPEC	__declspec(dllexport)
#   else
#    define DECLSPEC
#   endif
#  else
#   define DECLSPEC
#  endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define DECLSPEC	__attribute__ ((visibility("default")))
#  else
#   define DECLSPEC
#  endif
# endif
# define _AGAR_DEFINED_DECLSPEC
#endif
#ifdef __SYMBIAN32__ 
# ifndef EKA2 
#  undef DECLSPEC
#  define DECLSPEC
#  define _AGAR_DEFINED_DECLSPEC
# elif !defined(__WINS__)
#  undef DECLSPEC
#  define DECLSPEC __declspec(dllexport)
#  define _AGAR_DEFINED_DECLSPEC
# endif
#endif

/*
 * Force structure packing at 4 byte alignment. This is necessary if the
 * header is included in code which has structure packing set to an alternate
 * value. The packing is reset to the previous value in close.h.
 */
#if defined(_MSC_VER) || defined(__MWERKS__) || defined(__BORLANDC__)
# ifdef _MSC_VER
#  pragma warning(disable: 4103)
# endif
# ifdef __BORLANDC__
#  pragma nopackwarning
# endif
# pragma pack(push,4)
#elif (defined(__MWERKS__) && defined(__MACOS__))
# pragma options align=mac68k4byte
# pragma enumsalwaysint on
#endif

/*
 * Expand "__inline__" to any compiler-specific keyword needed for defining
 * an inline function, if supported.
 */
#ifndef AG_INLINE_OKAY
# ifdef __GNUC__
#  define AG_INLINE_OKAY
# else
#  if defined(_MSC_VER) || defined(__BORLANDC__) || \
      defined(__DMC__) || defined(__SC__) || \
      defined(__WATCOMC__) || defined(__LCC__) || \
      defined(__DECC) || defined(__EABI__)
#   ifndef __inline__
#    define __inline__	__inline
#   endif
#   define AG_INLINE_OKAY
#  else
#   if !defined(__MRC__) && !defined(_SGI_SOURCE)
#    ifndef __inline__
#     define __inline__ inline
#    endif
#    define AG_INLINE_OKAY
#   endif
#  endif
# endif
#endif
#ifndef AG_INLINE_OKAY
# define __inline__
#endif
#undef AG_INLINE_OKAY

/* Define NULL if needed. */
#if !defined(NULL) && !defined(__MACH__)
# ifdef __cplusplus
#  define NULL 0
#  define _AGAR_DEFINED_NULL
# else
#  define NULL ((void *)0)
#  define _AGAR_DEFINED_NULL
# endif
#endif
