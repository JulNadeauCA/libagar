/*	$Csoft: world.h,v 1.23 2002/11/27 05:05:30 vedge Exp $	*/
/*	Public domain	*/

struct world {
	struct	object obj;

	/* Read-write, thread-safe */
	SLIST_HEAD(, object)	wobjs;	/* Game objects */
	int			nobjs;

	/* Lock on the object list. */
	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
};

extern struct world *world;	/* Consistent while running */

void	 world_init(struct world *, char *);
void	 world_destroy(void *);
int	 world_load(void *, int);
int	 world_save(void *, int);
void	 world_attach(void *);
void	 world_detach(void *);

int	 world_attached(void *);
void	*world_find(char *);

