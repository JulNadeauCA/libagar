/*	$Csoft: engine.h,v 1.40 2002/11/07 18:52:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#define ENGINE_VERSION	"1.0-beta"

#include <engine/mcconfig.h>
#if !defined(__OpenBSD__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 500	/* XXX recursive mutexes */
#endif
#include <pthread.h>		/* For pthread types */
#include <SDL.h>		/* For SDL types */
#include <engine/compat/queue.h> /* For queue(3) definitions */
#include <engine/error.h>	/* Wrappers and error messages */
#include <engine/debug.h>	/* Debugging macros */
#include <engine/object.h>	/* Most structures are derived from this */
#include <engine/event.h>	/* For event handler prototypes */
#include <engine/anim.h>	/* XXX */
#include <engine/world.h>	/* XXX */
#include <engine/view.h>	/* XXX */

#if !defined(SERIALIZATION)
#define pthread_mutex_init(mu, attr)
#define pthread_mutex_destroy(mu)
#define pthread_mutex_lock(mu)
#define pthread_mutex_trylock(mu)
#define pthread_mutex_unlock(mu)
#endif

struct gameinfo {
	char	*prog;
	char	*name;
	char	*copyright;
	char	*version;
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

#endif /* !_AGAR_ENGINE_H_ */
