/*	$Csoft: engine.h,v 1.52 2003/01/01 01:43:42 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#define ENGINE_VERSION	"1.0-beta"

#include <config/debug.h>
#include <config/serialization.h>
#include <config/have_opengl.h>

#if !defined(__OpenBSD__)
# ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 500	/* XXX recursive mutexes, pread and pwrite */
# endif
#endif

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef SERIALIZATION
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
#include <libfobj/buf.h>	 /* For the fobj_buf type */
#include <engine/error.h>	 /* Wrappers and error messages */
#include <engine/debug.h>	 /* Debugging macros */
#include <engine/object.h> 	 /* Most structures are derived from this */
#include <engine/event.h>	 /* For event handler prototypes */

#ifdef SERIALIZATION
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
#endif

struct engine_proginfo {
	char	*prog;		/* Name of the executable */
	char	*name;		/* Name of the game */
	char	*copyright;	/* Copyright notice */
	char	*version;	/* Version of the game */
};

enum {
	ICON_GAME,
	ICON_MAPEDITION
};

#define ENGINE_INIT_GFX		0x01		/* Graphic engine */
#define ENGINE_INIT_TEXT	0x02		/* Font engine */
#define ENGINE_INIT_INPUT	0x04		/* Input devices */

int	 engine_init(int, char **, const struct engine_proginfo *, int);
void	 engine_stop(void);
void	 engine_destroy(void);

#endif	/* !_AGAR_ENGINE_H_ */
