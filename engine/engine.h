/*	$Csoft: engine.h,v 1.2 2002/01/30 17:47:43 vedge Exp $	*/

#include <pthread.h>

#include <SDL.h>
#include <glib.h> /* XXX temporary */

#include <libfobj/fobj.h>

#include <engine/queue.h>
#include <engine/debug.h>

#include <engine/anim.h>
#include <engine/object.h>
#include <engine/world.h>

#include <engine/view.h>
#include <engine/xcf.h>
#include <engine/map.h>
#include <engine/event.h>
#include <engine/char.h>

#define ENGINE_VERSION	"0.1"

struct gameinfo {
	char	*prog;
	char	*name;
	char	*copyright;
	int	 ver[2];
};

int	engine_init(int, char **, struct gameinfo *);
int	engine_mapedit(void);
void	engine_destroy(void);
void	engine_join(void);

