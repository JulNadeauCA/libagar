/*	$Csoft: engine.h,v 1.49 2002/12/24 10:27:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#define ENGINE_VERSION	"1.0-beta"

#include <config/debug.h>
#include <config/serialization.h>

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
#else
# define pthread_mutex_t	int
# define pthread_mutexattr_t	int
# define pthread_t		int
#endif

#include <SDL.h>		 /* For SDL types */

#include <engine/compat/queue.h> /* For queue(3) definitions */

#include <libfobj/buf.h>	 /* For the fobj_buf type */

#include <engine/error.h>	 /* Wrappers and error messages */
#include <engine/debug.h>	 /* Debugging macros */
#include <engine/object.h> 	 /* Most structures are derived from this */
#include <engine/event.h>	 /* For event handler prototypes */

#ifdef SERIALIZATION
# ifdef _SGI_SOURCE
/* Irix <pthread.h> defines PTHREAD_MUTEX_INITIALIZER as { 0 }. */
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

struct gameinfo {
	char	*prog;		/* Name of the executable */
	char	*name;		/* Name of the game */
	char	*copyright;	/* Copyright notice */
	char	*version;	/* Version of the game */
};

enum {
	ICON_GAME,
	ICON_MAPEDITION
};

/* engine_start() return value. XXX */
enum {
	ENGINE_START_GAME,
	ENGINE_START_MAP_EDITION
};

/* engine_init() flags. XXX */
#define ENGINE_INIT_GUI		0x01	/* Use only GUI engine */
#define ENGINE_INIT_MAPEDIT	0x02	/* Add map editor switch (-e) */

int	 engine_init(int, char **, const struct gameinfo *, char *, int);
int	 engine_start(void);
void	 engine_stop(void);
void	 engine_destroy(void);

#endif	/* !_AGAR_ENGINE_H_ */
