/*	$Csoft: engine.h,v 1.11 2002/03/03 06:22:34 vedge Exp $	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#include <engine/config.h>

#include <pthread.h>

#include <SDL.h>

#include <libfobj/fobj.h>

#include <engine/queue.h>
#include <engine/debug.h>
#include <engine/event.h>
#include <engine/anim.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/xcf.h>
#include <engine/map.h>
#include <engine/view.h>
#include <engine/char.h>

#define ENGINE_VERSION	"0.1"

struct gameinfo {
	char	*prog;
	char	*name;
	char	*copyright;
	int	 ver[2];
};

int	engine_init(int, char **, struct gameinfo *, char *);
int	engine_mapedit(void);
void	engine_destroy(void);
void	engine_editmap(void);

void	*emalloc(size_t);

#endif	/* _AGAR_ENGINE_H_ */
