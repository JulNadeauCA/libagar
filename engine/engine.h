/*	$Csoft: engine.h,v 1.61 2003/04/26 06:19:52 vedge Exp $	*/
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
# if !defined(__OpenBSD__)
#  ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 500	/* XXX for recursive mutexes */
#  endif
# endif
# include <pthread.h>		/* For pthread types */
# include <signal.h>		/* For pthread_kill() */
#else
# define pthread_mutex_t	int
# define pthread_mutexattr_t	int
# define pthread_t		int
#endif

#include <SDL.h>		 /* For SDL types */
#ifdef HAVE_OPENGL
#include <GL/gl.h>		 /* For OpenGL types */
#endif

#include <engine/compat/queue.h> /* For queue(3) definitions */
#include <libfobj/fobj.h>	 /* For struct netbuf */
#include <engine/error.h>	 /* Wrappers and error messages */
#include <engine/debug.h>	 /* Debugging macros */
#include <engine/object.h> 	 /* Most structures are derived from this */
#include <engine/event.h>	 /* For event handler prototypes */

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
extern DECLSPEC void	 engine_stop(void);
extern DECLSPEC void	 engine_destroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* !_AGAR_ENGINE_H_ */
