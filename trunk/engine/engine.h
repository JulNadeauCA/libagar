/*	$Csoft: engine.h,v 1.66 2003/06/15 05:04:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#define ENGINE_VERSION	"1.0-beta"

#include <config/debug.h>
#include <config/threads.h>
#include <config/have_opengl.h>

#include <sys/types.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

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

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif

#include <engine/compat/queue.h>
#include <engine/compat/strlcpy.h>
#include <engine/compat/strlcat.h>
#include <engine/compat/snprintf.h>
#include <engine/compat/vsnprintf.h>
#include <engine/compat/vasprintf.h>
#include <engine/compat/strsep.h>

#include <libfobj/fobj.h>

#include <engine/error.h>
#include <engine/debug.h>
#include <engine/object.h>
#include <engine/event.h>

#include <engine/unicode/unicode.h>
#include <engine/widget/text.h>

/* Preparation for m17n. */
#undef _
#undef N_
#undef textdomain
#undef bindtextdomain
#define _(s) (s)
#define N_(s) (s)
#define textdomain(d)
#define bindtextdomain(p, d)

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

#include "begin_code.h"

struct engine_proginfo {
	char	*progname;	/* Name of the executable */
	char	*name;		/* Name of the game */
	char	*copyright;	/* Copyright notice */
	char	*version;	/* Version of the game */
};

extern struct engine_proginfo *proginfo;	/* engine.c */

enum {
	ICON_GAME,
	ICON_MAPEDITION
};

#define ENGINE_INIT_GFX		0x01		/* Graphic engine */
#define ENGINE_INIT_INPUT	0x02		/* Input devices */

extern struct object *world;	/* Roots of Evil */

__BEGIN_DECLS
extern DECLSPEC int	 engine_init(int, char **, struct engine_proginfo *,
			             int);
extern DECLSPEC void	 engine_destroy(void);
extern __inline__ void	 lock_linkage(void);
extern __inline__ void	 unlock_linkage(void);
__END_DECLS

#include "close_code.h"
#endif	/* !_AGAR_ENGINE_H_ */
