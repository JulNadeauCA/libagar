/*	$Csoft: map.h,v 1.12 2002/02/14 05:26:49 vedge Exp $	*/

#define MAP_WIDTH	256
#define MAP_HEIGHT	256

#define MAP_MAGIC	"agar map  "
#define MAP_VERMAJ	1
#define MAP_VERMIN	8

struct map_aref {
	struct	object *pobj;	/* Object pointer */
	int	offs;		/* Sprite/anim within this object */
	int	flags;
#define MAPREF_SAVE	0x0001	/* Map dumps must record this reference */
#define MAPREF_SPRITE	0x0002	/* This is a sprite */
#define MAPREF_ANIM	0x0004	/* this is sequence of sprites */
#define MAPREF_WARP	0x0008	/* This is another map */

	int	frame;		/* Animation frame # */
	int	fwait;		/* Animation delay counter */

	int	xoffs, yoffs;	/* Incremented if > 0, decremented if < 0,
				   used for direction and soft scroll. */

	TAILQ_ENTRY(map_aref) marefs;	/* Map entry reference list */
};

/* Back reference to map:x,y coordinate. */
struct map_bref {
	struct	map *map;	/* Map structure */
	int	x, y;		/* X:Y coordinate */
};

TAILQ_HEAD(map_arefs_head, map_aref);

/* Coordinate within a map. */
struct node {
	struct	map_arefs_head arefsh;
	int	narefs;
	int	flags;
#define NODE_BLOCK	0x0000		/* Cannot walk through */
#define NODE_ORIGIN	0x0001		/* Origin of this map */
#define NODE_WALK	0x0002		/* Can walk through */
#define NODE_CLIMB	0x0004		/* Can climb (eg. ladder) */
#define NODE_SLIP	0x0008		/* Slippery */
#define NODE_BIO	0x0010		/* Cause Poison */
#define NODE_REGEN	0x0020		/* Cause HP Regeneration */
#define NODE_SLOW	0x0040		/* Decrease speed by v1 */
#define NODE_HASTE	0x0080		/* Increase speed by v1 */
#define NODE_ANIM	0x0100		/* One anim at least (optimization) */
#define NODE_DONTSAVE	(NODE_ANIM)
	int	v1, v2;			/* Extra properties */
	int	nanims;			/* Animation count */
};


/* Region within the world. */
struct map {
	struct	object obj;
	struct	viewport *view;
	int 	flags;
#define MAP_FOCUSED	0x0001		/* Being displayed */
#define MAP_VARTILEGEO	0x0010		/* Variable tile geometry */
#define MAP_2D		0x0020		/* Two-dimensional */
	int	mapw, maph;
	int	defx, defy;
	struct	node map[MAP_WIDTH][MAP_HEIGHT];
	int	redraw;

	SDL_TimerID timer;		/* Map display timer */
	pthread_mutex_t lock;		/* Lock on map entry reference lists */
	
	SLIST_ENTRY(map) wmaps;		/* Active maps */
};

extern struct map *curmap;	/* Currently focused map */

/* Add a sprite reference to ob:offs at m:x,y */
#define MAP_ADDSPRITE(m, x, y, ob, soffs)			\
	node_addref(&(m)->map[(x)][(y)],			\
	    (ob), (soffs), MAPREF_SPRITE);			\

/* Add an animation reference to ob:offs at m:x,y */
#define MAP_ADDANIM(m, x, y, ob, aoffs)				\
	node_addref(&(m)->map[(x)][(y)],			\
	    (ob), (aoffs), MAPREF_ANIM);			\

/*
 * Drop an animation/sprite reference to ob:offs at m:x,y. If offs is
 * negative, drop all references to ob:*.
 */
#define MAP_DELREF(m, x, y, ob, offs)				\
	do {							\
		struct node *me = &(m)->map[(x)][(y)];	\
		struct map_aref *maref;				\
								\
		while ((maref = node_arefobj((me),		\
		    (ob), (offs)))) {				\
			node_delref((me), (maref));		\
		}						\
	} while (/*CONSTCOND*/ 0)

struct map *map_create(char *, char *, int, int, int);

void	map_destroy(void *);
int	map_load(void *, char *);
int	map_save(void *, char *);
int	map_focus(struct map *);
int	map_unfocus(struct map *);
void	map_clean(struct map *, struct object *, int, int, int);
int	map_animnode(struct map *, int x, int y);
int	map_unanimnode(struct map *, int x, int y);
#ifdef DEBUG
void	map_dump(struct map *);
#endif

struct map_aref *node_addref(struct node *, struct object *, int, int);
struct map_aref	*node_arefindex(struct node *, int);
struct map_aref	*node_arefobj(struct node *, struct object *, int);
int		 node_delref(struct node *, struct map_aref *);

struct map_aref *node_popref(struct node *);
int		 node_pushref(struct node *, struct map_aref *);

