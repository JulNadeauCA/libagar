/*	$Csoft: world.h,v 1.20 2002/06/25 17:59:51 vedge Exp $	*/
/*	Public domain	*/

struct world {
	struct	object obj;

	/* Read-write, thread-safe */
	SLIST_HEAD(, object) wobjsh;	/* Game objects */
	int	nobjs;
	pthread_mutex_t lock;
};

extern struct world *world;	/* Consistent while running */

void	 world_init(struct world *, char *);
void	 world_destroy(void *);
int	 world_load(void *, int);
int	 world_save(void *, int);
void	 world_attach(void *, void *);
void	 world_detach(void *, void *);

