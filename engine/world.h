/*	$Csoft$	*/

#include <glib.h>

struct world {
	struct	object obj;
	int	laws;			/* Laws governing this world */
	float	agef;			/* Global ageing factor */
	int	nobjs;			/* Object count */
	int	nchars;			/* Inhabitant count */
	int	nmaps;			/* Map count */
	GSList *objs;			/* Objects (struct object) */
	GSList *chars;			/* Characters (struct character) */
	GSList *maps;			/* Mapped areas (struct map) */
	pthread_mutex_t lock;		/* R/W lock on object lists */
};

extern struct world *world;

extern struct world *world_create(char *);
extern void	     world_destroy(struct object *);
extern void	     quit(void);
#ifdef DEBUG
extern void	     world_dump(struct world *);
#endif

