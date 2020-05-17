/*	Public domain	*/

#ifdef _AGAR_MICRO_BEGIN_H_
#error Nested inclusion of <agar/micro/begin.h>
#endif
#define _AGAR_MICRO_BEGIN_H_

/* Primitive types */
#include <agar/core/types.h>

/* Compiler-specific attributes and annotations */
#include <agar/core/attributes.h>

/* Declarations */
#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# define _AGAR_MICRO_DEFINED_CDECLS
# if defined(__cplusplus)
#  define __BEGIN_DECLS extern "C" {
#  define __END_DECLS   }
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif
#ifndef DECLSPEC
#define DECLSPEC
#endif

/* Inlining (haha) */
#define __inline__

/* Nullability */
#if !defined(NULL) && !defined(__MACH__)
# ifdef __cplusplus
#  define NULL 0
#  define _AGAR_MICRO_DEFINED_NULL
# else
#  define NULL ((void *)0)
#  define _AGAR_MICRO_DEFINED_NULL
# endif
#endif
#if defined(__GNUC__) || defined(__CC65__)
# define _Nonnull
# define _Nullable
# define _Null_unspecified
# define _AGAR_MICRO_DEFINED_NULLABILITY
#else
# include <agar/micro/nullability.h>
#endif

/* Restrict */
#if !defined(_Restrict)
# if !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
#  if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901 || defined(lint)
#   define _Restrict
#  else
#   define _Restrict restrict
#  endif
#  define _AGAR_MICRO_DEFINED_RESTRICT
# endif
#endif

