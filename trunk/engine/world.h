/*	$Csoft: world.h,v 1.2 2002/02/03 11:21:43 vedge Exp $	*/

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
extern void	     quit(void);
#ifdef DEBUG
extern void	     world_dump(struct world *);
#endif

