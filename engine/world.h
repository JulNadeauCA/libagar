/*	$Csoft: world.h,v 1.25 2003/01/31 03:03:29 vedge Exp $	*/
/*	Public domain	*/

struct world {
	struct object	obj;

	pthread_mutex_t		lock;
	SLIST_HEAD(, object)	wobjs;		/* Game objects */
	int			nobjs;
};

extern struct world *world;	/* Consistent while running */

void	 world_init(struct world *, char *);
void	 world_destroy(void *);
int	 world_load(void *, struct netbuf *);
int	 world_save(void *, struct netbuf *);
void	 world_attach(void *);
void	 world_detach(void *);

int	 world_attached(void *);
void	*world_find(char *);

