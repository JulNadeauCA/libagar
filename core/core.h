/*	$Csoft: engine.h,v 1.102 2005/10/02 09:41:08 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

#include <agar/config/have_opengl.h>
#include <agar/config/enable_nls.h>
#include <agar/config/threads.h>
#include <agar/config/floating_point.h>
#include <agar/config/edition.h>
#include <agar/config/network.h>
#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#include <agar/config/map.h>
#include <agar/config/bsd_types_needed.h>
#include <agar/config/have_sys_types_h.h>

#include <agar/core/threads.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef BSD_TYPES_NEEDED
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned long u_long;
#endif

#include <SDL.h>
#include <SDL_endian.h>

#ifdef HAVE_OPENGL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
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

#include <agar/core/error.h>

#include <agar/compat/queue.h>
#include <agar/compat/strlcpy.h>
#include <agar/compat/strlcat.h>
#include <agar/compat/snprintf.h>
#include <agar/compat/vsnprintf.h>
#include <agar/compat/asprintf.h>
#include <agar/compat/vasprintf.h>
#include <agar/compat/strsep.h>
#include <agar/compat/math.h>

#include <agar/core/loaders/netbuf.h>
#include <agar/core/loaders/integral.h>
#include <agar/core/loaders/real.h>
#include <agar/core/loaders/string.h>
#include <agar/core/loaders/version.h>
#include <agar/core/loaders/color.h>

#include <agar/core/object.h>
#include <agar/core/event.h>
#include <agar/core/icons.h>

#include <agar/gui/text.h>

#ifdef ENABLE_NLS
# include <agar/libintl/libintl.h>
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

#ifdef WIN32
#define AG_PATHSEPC '\\'
#define AG_PATHSEP "\\"
#else
#define AG_PATHSEPC '/'
#define AG_PATHSEP "/"
#endif

#include <agar/core/core_init.h>
#endif /* !_AGAR_CORE_CORE_H_ */
