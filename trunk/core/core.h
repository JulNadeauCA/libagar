/*	Public domain	*/

#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

#include <config/have_opengl.h>
#include <config/enable_nls.h>
#include <config/threads.h>
#include <config/edition.h>
#include <config/network.h>
#include <config/have_bounded_attribute.h>
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>
#include <config/have_64bit.h>

#include <config/_mk_have_unsigned_typedefs.h>
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned char Uchar;
typedef unsigned long Ulong;
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

#ifdef HAVE_OPENGL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
# else
#  include <GL/gl.h>
#  include <GL/glu.h>
# endif
#endif

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

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#include <core/error.h>

#include <config/_mk_have_sys_queue_h.h>
#ifdef _MK_HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#else
#include <core/queue.h>
#endif

#include <core/strlcpy.h>
#include <core/strlcat.h>
#include <core/snprintf.h>
#include <core/vsnprintf.h>
#include <core/asprintf.h>
#include <core/vasprintf.h>
#include <core/strsep.h>
#include <core/math.h>

#include <core/data_source.h>
#include <core/load_integral.h>
#include <core/load_real.h>
#include <core/load_string.h>
#include <core/load_version.h>
#include <core/load_color.h>

#include <core/object.h>
#include <core/event.h>
#include <core/cpuinfo.h>
#include <core/typesw.h>

#include <gui/text.h>

#ifdef ENABLE_NLS
# include <libintl/libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# undef textdomain
# undef bindtextdomain
# define _(s) (s)
# define N_(s) (s)
# define textdomain(d)
# define bindtextdomain(p, d)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifdef WIN32
#define AG_PATHSEPC '\\'
#define AG_PATHSEP "\\"
#else
#define AG_PATHSEPC '/'
#define AG_PATHSEP "/"
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4018)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif

#include <core/core_init.h>
#endif /* !_AGAR_CORE_CORE_H_ */
