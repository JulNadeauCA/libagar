/*	$Csoft: world.h,v 1.11 2002/04/24 13:18:20 vedge Exp $	*/

struct world {
	struct	object obj;

	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */
	struct	map *curmap;		/* Map being displayed */

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

