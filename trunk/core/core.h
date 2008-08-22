/*	Public domain	*/
/*
 * Main internal Agar-Core header file. External applications and libraries
 * should use <agar/core.h> instead.
 */

#ifdef _AGAR_INTERNAL
#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

#include <config/debug.h>
#include <config/lockdebug.h>
#include <config/threads.h>
#include <config/network.h>
#include <config/have_bounded_attribute.h>
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>
#include <config/have_64bit.h>
#include <config/have_long_double.h>
#include <config/enable_nls.h>

#include <config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned char Uchar;
typedef unsigned long Ulong;
#endif

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#include <core/threads.h>

#include <SDL.h>
#include <SDL_endian.h>

#include <config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <config/_mk_have_unistd_h.h>
#ifdef _MK_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>

#ifdef HAVE_BOUNDED_ATTRIBUTE
#define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
#define BOUNDED_ATTRIBUTE(t, a, b)
#endif
#ifdef HAVE_FORMAT_ATTRIBUTE
#define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
#define FORMAT_ATTRIBUTE(t, a, b)
#endif
#ifdef HAVE_NONNULL_ATTRIBUTE
#define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
#define NONNULL_ATTRIBUTE(a)
#endif

#define AG_BIG_ENDIAN 4321
#define AG_LITTLE_ENDIAN 1234
#include <config/_mk_big_endian.h>
#include <config/_mk_little_endian.h>
#if defined(_MK_BIG_ENDIAN)
# define AG_BYTEORDER AG_BIG_ENDIAN
#elif defined(_MK_LITTLE_ENDIAN)
# define AG_BYTEORDER AG_LITTLE_ENDIAN
#else
# error "Byte order is unknown"
#endif
#undef _MK_BIG_ENDIAN
#undef _MK_LITTLE_ENDIAN

#include <core/core_init.h>
#include <core/error.h>
#include <core/queue.h>
#include <core/limits.h>

#include <core/string_compat.h>
#include <core/snprintf.h>
#include <core/vsnprintf.h>
#include <core/vasprintf.h>

#ifdef WIN32
#define AG_PATHSEPC '\\'
#define AG_PATHSEP "\\"
#else
#define AG_PATHSEPC '/'
#define AG_PATHSEP "/"
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN3
#define	MIN3(a,b,c) MIN((a),MIN((b),(c)))
#endif
#ifndef MAX3
#define	MAX3(a,b,c) MAX((a),MAX((b),(c)))
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4018)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif

#include <core/data_source.h>
#include <core/load_integral.h>
#include <core/load_real.h>
#include <core/load_string.h>
#include <core/load_version.h>
#include <core/load_color.h>

#include <core/version.h>
#include <core/object.h>
#include <core/cpuinfo.h>
#include <core/file.h>
#include <core/dir.h>
#include <core/dso.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(String) dgettext("agar",String)
# ifdef dgettext_noop
#  define N_(String) dgettext_noop("agar",String)
# else
#  define N_(String) (String)
# endif
#else
# undef _
# undef N_
# undef ngettext
# define _(String) (String)
# define N_(String) (String)
# define ngettext(Singular,Plural,Number) ((Number==1)?(Singular):(Plural))
#endif /* !ENABLE_NLS */

#endif /* !_AGAR_CORE_CORE_H_ */
#endif /* _AGAR_INTERNAL */
