/*	$Csoft: engine.h,v 1.21 2002/05/19 14:31:30 vedge Exp $	*/

#ifndef _AGAR_ENGINE_H_
#define _AGAR_ENGINE_H_

#include <engine/mcconfig.h>

#include <pthread.h>
#include <stdlib.h>

#include <SDL.h>

#include <libfobj/fobj.h>

#include <engine/queue.h>
#include <engine/debug.h>
#include <engine/event.h>
#include <engine/anim.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/xcf.h>
#include <engine/view.h>

#define ENGINE_VERSION	"1.0-beta"

struct gameinfo {
	char	*prog;
	char	*name;
	char	*copyright;
	int	 ver[2];
};


#define ICON_GAME	0
#define ICON_MAPEDITION	1

int		 engine_init(int, char **, struct gameinfo *, char *);
int		 engine_editmap(void);
void		*emalloc(size_t);
void		*erealloc(void *, size_t);
void		 engine_destroy(void);

#endif	/* _AGAR_ENGINE_H_ */
