/*	$Csoft: map.h,v 1.16 2002/02/15 10:49:28 vedge Exp $	*/

#define MAP_MAGIC	"agar map  "
#define MAP_VERMAJ	1
#define MAP_VERMIN	9

struct noderef {
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

	TAILQ_ENTRY(noderef) nrefs;	/* Node reference list */
};

/* Back reference to map:x,y coordinate. */
struct map_bref {
	struct	map *map;	/* Map structure */
	int	x, y;		/* X:Y coordinate */
};

TAILQ_HEAD(noderefs_head, noderef);

/* Coordinate within a map. */
struct node {
	struct	noderefs_head nrefsh;
	int	nnrefs;
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

	int	redraw;			/* Redraw at next tick */
	int	mapw, maph;		/* Map geometry */
	int	tilew, tileh;		/* Tile geometry */
	int	shtilex, shtiley;	/* Tile shift (optimization) */
	int	defx, defy;		/* Map origin */
	struct	node **map;		/* Array of nodes */

	pthread_t	draw_th;	/* Map rendering thread */
	pthread_mutex_t lock;		/* Lock on all nodes */
	
	SLIST_ENTRY(map) wmaps;		/* Active maps */
};

extern struct map *curmap;	/* Currently focused map */

struct map	*map_create(char *, char *, int);
void		 map_destroy(void *);
int		 map_load(void *, char *);
int		 map_save(void *, char *);
int		 map_focus(struct map *);
int		 map_unfocus(struct map *);
void		 map_clean(struct map *, struct object *, int, int, int);
void		 map_allocnodes(struct map *, int, int, int, int);
void		 map_freenodes(struct map *);
int		 map_animnode(struct map *, int x, int y);
int		 map_unanimnode(struct map *, int x, int y);
void		 map_plot_sprite(struct map *, SDL_Surface *, int, int);
#ifdef DEBUG
void		 map_dump(struct map *);
#endif

struct noderef	*node_addref(struct node *, void *, int, int);
struct noderef	*node_findref(struct node *, void *, int);
int		 node_delref(struct node *, struct noderef *);

struct noderef	*node_popref(struct node *);
int		 node_pushref(struct node *, struct noderef *);

