/*	$Csoft: world.h,v 1.8 2002/02/25 09:07:53 vedge Exp $	*/

struct world {
	struct	object obj;

	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */
	struct	map *curmap;		/* Map being displayed */

	SLIST_HEAD(, object) wobjsh;	/* Active objects */
	SLIST_HEAD(, character) wcharsh; /* Active characters */
	pthread_mutex_t lock;		/* R/W lock on lists */
};

extern struct world *world;

struct world	*world_create(char *);
int		 world_load(void *, int);
int		 world_save(void *, int);
int		 world_destroy(void *);
void		 world_dump(void *);

char		*savepath(char *, const char *);

