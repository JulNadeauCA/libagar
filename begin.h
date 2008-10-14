/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/*
 * This file sets things up for C dynamic library function definitions,
 * static inlined functions, and structures aligned at 4-byte alignment.
 * If you don't like ugly C preprocessor code, don't look at this file. :)
 */

#ifdef _AGAR_BEGIN_H_
#error Nested inclusion of <agar/begin.h>
#endif
#define _AGAR_BEGIN_H_

/* Some compilers use a special export keyword for dynamic libraries */
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

/*
 * By default Agar uses the C calling convention, but on OS/2, we use the
 * _System calling convention to be compatible with every compiler.
 */
#ifndef AGARCALL
# if defined(__WIN32__) && !defined(__GNUC__)
#  define AGARCALL __cdecl
# else
#  ifdef __OS2__
#   define AGARCALL _System
#  else
#   define AGARCALL
#  endif
# endif
# define _AGAR_DEFINED_AGARCALL
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
 * Set up compiler-specific options for inlining functions. If inline not
 * supported, turn static __inline__ functions into regular static functions.
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

/* Apparently this is needed by several Windows compilers */
#if !defined(__MACH__)
# ifndef NULL
#  ifdef __cplusplus
#   define NULL 0
#   define _AGAR_DEFINED_NULL
#  else
#   define NULL ((void *)0)
#   define _AGAR_DEFINED_NULL
#  endif
# endif /* NULL */
#endif /* ! Mac OS X - breaks precompiled headers */
