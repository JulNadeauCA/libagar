/*	$Csoft: world.h,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $	*/

#include <glib.h>

SLIST_HEAD(objs_head, object);
SLIST_HEAD(maps_head, map);
SLIST_HEAD(chars_head, character);

struct world {
	struct	object obj;
	float	agef;			/* Global ageing factor */

	struct	objs_head wobjsh;	/* Active objects */
	struct	maps_head wmapsh;	/* Active maps */
	struct	chars_head wcharsh;	/* Active characters */
	
	pthread_mutex_t lock;		/* R/W lock on object lists */
};

extern struct world *world;

extern struct world *world_create(char *);
extern void	     world_destroy(struct object *);
extern void	     quit(void);
#ifdef DEBUG
extern void	     world_dump(struct world *);
#endif

