/*	$Csoft: world.h,v 1.13 2002/05/13 06:51:13 vedge Exp $	*/

struct world {
	struct	object obj;

	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */
	struct	map *curmap;		/* Map being displayed */

	SLIST_HEAD(, object) wobjsh;	/* Active objects */
	int	nobjs;
	pthread_mutex_t lock;		/* Lock on the entire structure */
};

/* Consistent throughout the game. */
extern struct world *world;

void	 world_init(struct world *, char *);
void	 world_destroy(void *);
int	 world_load(void *, int);
int	 world_save(void *, int);
void	 world_attach(void *, void *);
void	 world_detach(void *, void *);

char	*savepath(char *, const char *);

