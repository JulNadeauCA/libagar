/*	$Csoft: world.h,v 1.12 2002/05/11 04:06:56 vedge Exp $	*/

struct world {
	struct	object obj;

	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */
	struct	map *curmap;		/* Map being displayed */
	struct	viewport *curview;	/* Current view. XXX multi */

	SLIST_HEAD(, object) wobjsh;	/* Active objects */
	int	nobjs;
	SLIST_HEAD(, character) wcharsh; /* Active characters */
	int	nchars;
	pthread_mutex_t lock;		/* Lock on the entire structure */
};

extern struct world *world;

void	 world_init(struct world *, char *);
void	 world_destroy(void *);
int	 world_load(void *, int);
int	 world_save(void *, int);

char	*savepath(char *, const char *);

