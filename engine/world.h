/*	$Csoft: world.h,v 1.3 2002/02/07 05:17:21 vedge Exp $	*/

SLIST_HEAD(objs_head, object);
SLIST_HEAD(chars_head, character);

struct world {
	struct	object obj;
	float	agef;			/* Global ageing factor */

	struct	objs_head wobjsh;	/* Active objects */
	struct	chars_head wcharsh;	/* Active characters */
	
	pthread_mutex_t lock;		/* R/W lock on object lists */
};

extern struct world *world;

extern struct world *world_create(char *);
extern void	     quit(void);
#ifdef DEBUG
extern void	     world_dump(struct world *);
#endif

