/*	$Csoft: world.h,v 1.24 2003/01/16 04:07:05 vedge Exp $	*/
/*	Public domain	*/

struct world {
	struct object	obj;

	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;

	SLIST_HEAD(, object)	wobjs;		/* Game objects */
	int			nobjs;
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

