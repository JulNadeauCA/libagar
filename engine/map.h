/*	$Csoft$	*/

#define MAP_WIDTH	256
#define MAP_HEIGHT	256

#define MAP_MAGIC	"agar map  "
#define MAP_VERMAJ	1
#define MAP_VERMIN	8

/* Object reference, this forms a linked list within each map entry. */
struct map_aref {
	struct	object *pobj;	/* Object pointer */
	int	index;		/* Index within the linked list */
	int	offs;		/* Sprite/anim within this object */
	int	flags;
#define MAPREF_SAVE	0x0001	/* Map dumps must record this reference */
#define MAPREF_SPRITE	0x0002	/* This is a sprite */
#define MAPREF_ANIM	0x0004	/* This is sequence of sprites */
	int	frame;		/* Animation frame # */
	int	fwait;		/* Delay counter */
};

/* Coordinate within a map. */
struct map_entry {
	GSList	*objs;			/* Object references */
	int	nobjs;			/* Object count */
	int	flags;
#define MAPENTRY_BLOCK	0x0000		/* Cannot walk through */
#define MAPENTRY_ORIGIN	0x0001		/* Origin of this map */
#define MAPENTRY_WALK	0x0002		/* Can walk through */
#define MAPENTRY_CLIMB	0x0004		/* Can climb (eg. ladder) */
#define MAPENTRY_SLIP	0x0008		/* Slippery */
#define MAPENTRY_BIO	0x0010		/* Cause Poison */
#define MAPENTRY_REGEN	0x0020		/* Cause HP Regeneration */
#define MAPENTRY_SLOW	0x0040		/* Decrease speed by v1 */
#define MAPENTRY_HASTE	0x0080		/* Increase speed by v1 */
	int	v1;			/* Extra property */
	int	nanims;			/* Animation count (optimization) */
};

/* Region within the world. */
struct map {
	struct	object obj;
	struct	viewport *view;
	int 	flags;
#define MAP_VARTILEGEO	0x0001		/* Variable tile geometry */
#define MAP_2D		0x0002		/* Two-dimensional */
	int	mapw, maph;
	int	defx, defy;
	struct	map_entry map[MAP_WIDTH][MAP_HEIGHT];
	int	redraw;

	SDL_TimerID timer;		/* Map display timer */
	pthread_mutex_t lock;		/* Lock on map entry reference lists */
};

extern struct map *curmap;	/* Currently focused map */
extern int mapedit;		/* Map edition in progress */

struct map	*map_create(char *, char *, int, int, int,
		     struct viewport *, char *);
struct map_aref *map_entry_addref(struct map_entry *, struct object *,
		     int, int);
struct map_aref	*map_entry_aref(struct map_entry *, int);
struct map_aref	*map_entry_arefobj(struct map_entry *,
		     struct object *, int);

int	map_focus(struct map *);
int	map_link(void *);
int	map_entry_delref(struct map_entry *, struct map_aref *);
int	map_entry_moveref(struct map *, struct object *, int, int, int);
void	map_clean(struct map *, struct object *, int, int, int);
int	map_animset(struct map *, int);
#ifdef DEBUG
void	map_dump_map(void *, void *);
#endif

