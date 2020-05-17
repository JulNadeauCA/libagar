/*	Public domain	*/

#ifdef _AGAR_NET_BEGIN_H_
#error Nested inclusion of <agar/net/begin.h>
#endif
#define _AGAR_NET_BEGIN_H_

#include <agar/core/types.h>
#include <agar/core/attributes.h>

/* NLS */
#if !defined(_)
# include <agar/config/enable_nls.h>
# ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext("agar",String)
#  ifdef dgettext_noop
#   define N_(String) dgettext_noop("agar",String)
#  else
#   define N_(String) (String)
#  endif
#  define _AGAR_NET_DEFINED_NLS
# else
#  undef _
#  undef N_
#  undef ngettext
#  define _(String) (String)
#  define N_(String) (String)
#  define ngettext(Singular,Plural,Number) ((Number==1)?(Singular):(Plural))
#  define _AGAR_NET_DEFINED_NLS
# endif
#endif /* defined(_) */

/* Declarations */
#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# define _AGAR_NET_DEFINED_CDECLS
# if defined(__cplusplus)
#  define __BEGIN_DECLS extern "C" {
#  define __END_DECLS   }
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif
#ifndef DECLSPEC
# if defined(__BEOS__) || defined(__HAIKU__)
#  if defined(__GNUC__)
#   define DECLSPEC
#  else
#   define DECLSPEC	__declspec(export)
#  endif
# elif defined(__WIN32__)
#  ifdef __BORLANDC__
#   ifdef _AGAR_NET_INTERNAL
#    define DECLSPEC 
#   else
#    define DECLSPEC	__declspec(dllimport)
#   endif
#  else
#   define DECLSPEC	__declspec(dllexport)
#  endif
# elif defined(__OS2__)
#  ifdef __WATCOMC__
#   ifdef _AGAR_NET_INTERNAL
#    define DECLSPEC	__declspec(dllexport)
#   else
#    define DECLSPEC
#   endif
#  elif defined (__GNUC__) && __GNUC__ < 4
#   ifdef _AGAR_NET_INTERNAL
#    define DECLSPEC    __declspec(dllexport)
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
#endif
#ifdef __SYMBIAN32__ 
#ifndef EKA2 
#undef DECLSPEC
#define DECLSPEC
#elif !defined(__WINS__)
#undef DECLSPEC
#define DECLSPEC __declspec(dllexport)
#endif /* !EKA2 */
#endif /* __SYMBIAN32__ */

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

/* Inlining */
#ifdef __GNUC__
# define _AGAR_NET_USE_INLINE
#else
# if defined(_MSC_VER) || defined(__BORLANDC__) || \
     defined(__DMC__) || defined(__SC__) || \
     defined(__WATCOMC__) || defined(__LCC__) || \
     defined(__DECC) || defined(__CC_ARM)
#  define AG_INLINE  __inline
#  ifndef __inline__
#  define __inline__	__inline
#  endif
#  define _AGAR_NET_USE_INLINE
# else
#  if !defined(__MRC__) && !defined(_SGI_SOURCE) && !defined(HAVE_CC65)
#   ifndef __inline__
#   define __inline__ inline
#   endif
#   define _AGAR_NET_USE_INLINE
#  endif
# endif
#endif /* !__GNUC__ */
#ifndef _AGAR_NET_USE_INLINE
# define __inline__
#endif

/* Nullability */
#if !defined(NULL) && !defined(__MACH__) && !defined(__CC65__)
# ifdef __cplusplus
#  define NULL 0
#  define _AGAR_NET_DEFINED_NULL
# else
#  define NULL ((void *)0)
#  define _AGAR_NET_DEFINED_NULL
# endif
#endif
#if !defined(AG_NULL)
# if !defined(__MACH__)
#  ifdef __cplusplus
#   define AG_NULL 0
#  else
#   define AG_NULL ((void *)0)
#  endif
# else
#  define AG_NULL NULL
# endif
#endif

#if defined(__GNUC__) || defined(__CC65__)
# define _Nonnull
# define _Nullable
# define _Null_unspecified
# define _AGAR_NET_DEFINED_NULLABILITY
#else
# include <agar/net/nullability.h>
#endif

/* Restrict */
#if !defined(_Restrict)
# if !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
#  if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901 || defined(lint)
#   define _Restrict
#  else
#   define _Restrict restrict
#  endif
#  define _AGAR_NET_DEFINED_RESTRICT
# endif
#endif
