/*	$Csoft: world.h,v 1.19 2002/06/12 20:40:06 vedge Exp $	*/
/*	Public domain	*/

struct world {
	struct	object obj;

	/* Read-only while running */
	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */

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

