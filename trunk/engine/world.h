/*	$Csoft: world.h,v 1.6 2002/02/18 07:50:32 vedge Exp $	*/

SLIST_HEAD(objs_head, object);
SLIST_HEAD(chars_head, character);

struct world {
	struct	object obj;

	char	*datapath;		/* Search path for states */
	char	*udatadir;		/* User data directory path */
	char	*sysdatadir;		/* System-wide data directory path */
	struct	map *curmap;		/* Map being displayed */

	struct	objs_head wobjsh;	/* Active objects */
	struct	chars_head wcharsh;	/* Active characters */
	pthread_mutex_t lock;		/* R/W lock on lists */
};

extern struct world *world;

struct world	*world_create(char *);
int		 world_load(void *, int);
int		 world_save(void *, int);
int		 world_destroy(void *);
#ifdef DEBUG
void		 world_dump(struct world *);
#endif

char		*savepath(char *, const char *);

