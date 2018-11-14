/*
 * Compiler-specific attributes and annotations.
 */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)stdlib.h	8.5 (Berkeley) 5/19/95
 */

/* Test for Clang extensions */
#ifndef	__has_attribute
#define __has_attribute(x) 0
#endif
#ifndef	__has_extension
#define __has_extension __has_feature
#endif
#ifndef	__has_feature
#define __has_feature(x) 0
#endif
#ifndef	__has_include
#define __has_include(x) 0
#endif
#ifndef	__has_builtin
#define __has_builtin(x) 0
#endif

#undef _Aligned_Attribute
#undef _Alloc_Align_Attribute
#undef _Alloc_Size_Attribute
#undef _Alloc_Size2_Attribute
#undef _Const_Attribute
#undef DEPRECATED_ATTRIBUTE
#undef FORMAT_ATTRIBUTE
#undef _Malloc_Like_Attribute
#undef _Noreturn_Attribute
#undef _Packed_Attribute
#undef _Pure_Attribute
#undef _Pure_Attribute_If_Unthreaded
#undef _Section_Attribute
#undef _Unused_Variable_Attribute
#undef _Used_Variable_Attribute
#undef _Warn_Unused_Result
#undef _Weak_Attribute

#include <agar/config/ag_use_attributes.h>

#if defined(lint) || !defined(AG_USE_ATTRIBUTES)
# define _Aligned_Attribute(x)
# define _Alloc_Align_Attribute(x)
# define _Alloc_Size_Attribute(x)
# define _Alloc_Size2_Attribute(n,x)
# define _Const_Attribute
# define DEPRECATED_ATTRIBUTE
# define FORMAT_ATTRIBUTE(t,a,b)
# define _Malloc_Like_Attribute
# define _Noreturn_Attribute
# define _Packed_Attribute
# define _Pure_Attribute
# define _Pure_Attribute_If_Unthreaded
# define _Section_Attribute(x)
# define _Unused_Variable_Attribute
# define _Used_Variable_Attribute
# define _Warn_Unused_Result
# define _Weak_Attribute
#else
# include <agar/config/have_aligned_attribute.h>
# include <agar/config/have_const_attribute.h>
# include <agar/config/have_deprecated_attribute.h>
# include <agar/config/have_format_attribute.h>
# include <agar/config/have_malloc_attribute.h>
# include <agar/config/have_noreturn_attribute.h>
# include <agar/config/have_packed_attribute.h>
# include <agar/config/have_pure_attribute.h>
# include <agar/config/have_warn_unused_result_attribute.h>
# include <agar/config/have_unused_variable_attribute.h>

# define _Weak_Attribute __attribute__((__weak__))

# ifdef HAVE_ALIGNED_ATTRIBUTE
#  undef  _Aligned_Attribute
#  define _Aligned_Attribute(x) __attribute__((__aligned__(x)))
# endif
# ifdef HAVE_CONST_ATTRIBUTE
#  undef  _Const_Attribute
#  define _Const_Attribute      __attribute__((__const__))
# endif
# ifdef HAVE_DEPRECATED_ATTRIBUTE
#  define DEPRECATED_ATTRIBUTE __attribute__((__deprecated__))
# else
#  define DEPRECATED_ATTRIBUTE
# endif
# ifdef HAVE_FORMAT_ATTRIBUTE
#  undef  FORMAT_ATTRIBUTE
#  define FORMAT_ATTRIBUTE(t,a,b) __attribute__((__format__ (t,a,b)))
# endif
# ifdef HAVE_MALLOC_ATTRIBUTE
#  define _Malloc_Like_Attribute __attribute__((__malloc__))
# else
#  define _Malloc_Like_Attribute
# endif
# ifdef HAVE_NORETURN_ATTRIBUTE
#  undef  _Noreturn_Attribute
#  define _Noreturn_Attribute __attribute__((__noreturn__))
# endif
# ifdef HAVE_PACKED_ATTRIBUTE
#  undef  _Packed_Attribute
#  define _Packed_Attribute __attribute__((__packed__))
# endif
# ifdef HAVE_PURE_ATTRIBUTE
#  define _Pure_Attribute __attribute__((__pure__))
# else
#  define _Pure_Attribute
# endif
# ifdef AG_THREADS
#  define _Pure_Attribute_If_Unthreaded
# else
#  ifdef HAVE_PURE_ATTRIBUTE
#   define _Pure_Attribute_If_Unthreaded __attribute__((__pure__))
#  else
#   define _Pure_Attribute_If_Unthreaded
#  endif
# endif /* AG_THREADS */
# ifdef HAVE_WARN_UNUSED_RESULT_ATTRIBUTE
#  define _Warn_Unused_Result __attribute__((__warn_unused_result__))
# else
#  define _Warn_Unused_Result
# endif
# ifdef HAVE_UNUSED_VARIABLE_ATTRIBUTE
#  undef  _Unused_Variable_Attribute
#  define _Unused_Variable_Attribute __attribute__((__unused__))
# endif

#endif /* lint */

/* C99-specific `restrict' type qualifier */
#if !defined(_Restrict)
# if !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
#  if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901 || defined(lint)
#   define _Restrict
#  else
#   define _Restrict restrict
#  endif
# endif
#endif

/* Clang nullability qualifiers on types. */
#ifdef __CC65__
# define _Nonnull
# define _Nullable
# define _Null_unspecified
#  define _AGAR_CORE_DEFINED_NULLABILITY
#else
# if !(defined(__clang__) && __has_feature(nullability))
#  undef  _Nonnull
#  define _Nonnull
#  undef  _Nullable
#  define _Nullable
#  undef  _Null_unspecified
#  define _Null_unspecified
#  define _AGAR_CORE_DEFINED_NULLABILITY
# endif
#endif

#if defined(__CC65__) && !defined(__inline__)
# define __inline__
#endif
