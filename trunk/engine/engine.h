/*	$Csoft: engine.h,v 1.9 2002/02/25 08:58:37 vedge Exp $	*/

#include <engine/config.h>

#include <pthread.h>

#include <SDL.h>

#include <libfobj/fobj.h>

#include <engine/queue.h>
#include <engine/debug.h>

#include <engine/event.h>
#include <engine/anim.h>
#include <engine/object.h>
#include <engine/input.h>
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

