/*	$Csoft: engine.h,v 1.33 2002/08/23 05:16:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#include <engine/mcconfig.h>

#include <pthread.h>
#include <stdlib.h>

#include <SDL.h>

#include <engine/compat/queue.h>

#include <engine/error.h>
#include <engine/debug.h>
#include <engine/object.h>
#include <engine/event.h>
#include <engine/anim.h>
#include <engine/world.h>
#include <engine/oldxcf.h>
#include <engine/view.h>

#if !defined(SERIALIZATION)
#define pthread_mutex_init(mu, attr)
#define pthread_mutex_destroy(mu)
#define pthread_mutex_lock(mu)
#define pthread_mutex_trylock(mu)
#define pthread_mutex_unlock(mu)
#endif

#define ENGINE_VERSION	"1.0-beta"

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

/* engine_start() return value */
#define ENGINE_START_GAME		0
#define ENGINE_START_MAP_EDITION	1

/* engine_init() flags. XXX odd */
#define ENGINE_INIT_GUI		0x01	/* Use only GUI engine */
#define ENGINE_INIT_MAPEDIT	0x02	/* Add map editor switch (-e) */

int	 engine_init(int, char **, const struct gameinfo *, char *, int);
int	 engine_start(void);
void	 engine_stop(void);
void	 engine_destroy(void);

#endif	/* _AGAR_ENGINE_H_ */
