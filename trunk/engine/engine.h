/*	$Csoft: engine.h,v 1.80 2004/03/25 05:36:34 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#include <config/have_opengl.h>
#include <config/enable_nls.h>
#include <config/threads.h>
#include <config/floating_point.h>
#include <config/edition.h>
#include <config/have_bounded_attribute.h>
#include <config/have_format_attribute.h>
#include <config/have_nonnull_attribute.h>

#include <sys/types.h>

#ifdef THREADS
# define _XOPEN_SOURCE 500	/* Require recursive mutexes */
# include <pthread.h>
# include <signal.h>
# undef _XOPEN_SOURCE
#else
# define pthread_mutex_t	int
# define pthread_mutexattr_t	int
# define pthread_t		int
#endif

#include <SDL.h>
#include <SDL_endian.h>
#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif

#ifdef HAVE_BOUNDED_ATTRIBUTE
#define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
#define BOUNDED_ATTRIBUTE(t, a, b)
#endif
#ifdef HAVE_FORMAT_ATTRIBUTE
#define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
#define FORMAT_ATTRIBUTE(t, a, b) /* nothing */
#endif
#ifdef HAVE_NONNULL_ATTRIBUTE
#define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
#define NONNULL_ATTRIBUTE(a) /* nothing */
#endif

#include <engine/error/error.h>

#include <compat/queue.h>
#include <compat/strlcpy.h>
#include <compat/strlcat.h>
#include <compat/snprintf.h>
#include <compat/vsnprintf.h>
#include <compat/asprintf.h>
#include <compat/vasprintf.h>
#include <compat/strsep.h>
#include <compat/math.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>
#include <engine/loader/real.h>
#include <engine/loader/string.h>
#include <engine/loader/version.h>

#include <engine/object.h>
#include <engine/event.h>
#include <engine/icons.h>

#include <engine/unicode/unicode.h>
#include <engine/widget/text.h>

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

#ifdef THREADS
extern pthread_mutexattr_t	recursive_mutexattr;
# ifdef _SGI_SOURCE
#  undef PTHREAD_MUTEX_INITIALIZER
#  define PTHREAD_MUTEX_INITIALIZER { { 0 } }
# endif
#else
# define pthread_mutex_destroy(mu)
# define pthread_mutex_init(mu, attr)
# define pthread_mutex_lock(mu)
# define pthread_mutex_trylock(mu)
# define pthread_mutex_unlock(mu)
# define pthread_mutexattr_init(mu)
# define pthread_mutexattr_destroy(mu)
# define pthread_mutexattr_settype(mu, type)
# define Pthread_create(th, attr, func, arg)
# define Pthread_join(th, ptr)
# define PTHREAD_MUTEX_INITIALIZER 0
# define PTHREAD_MUTEX_RECURSIVE 0
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#include "begin_code.h"

struct engine_proginfo {
	char	*progname;	/* Name of the executable */
	char	*name;		/* Name of the game */
	char	*copyright;	/* Copyright notice */
	char	*version;	/* Version of the game */
};

extern struct engine_proginfo *proginfo;	/* engine.c */

#define ENGINE_INIT_GFX		0x01		/* Graphic engine */
#define ENGINE_INIT_INPUT	0x02		/* Input devices */

extern struct object *world;		/* Roots of Evil */
extern pthread_mutex_t linkage_lock;	/* Protects object linkage */

__BEGIN_DECLS
int		 engine_init(int, char **, struct engine_proginfo *, int);
void		 engine_destroy(void);
__inline__ void	 lock_linkage(void);
__inline__ void	 unlock_linkage(void);
__END_DECLS

#include "close_code.h"
#endif	/* !_AGAR_ENGINE_H_ */
