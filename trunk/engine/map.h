/*	$Csoft: map.h,v 1.19 2002/02/25 09:01:54 vedge Exp $	*/

#define MAP_MAGIC	"agar map  "
#define MAP_VERMAJ	1
#define MAP_VERMIN	9

struct noderef {
	struct	object *pobj;	/* Object pointer */
	Uint32	offs;		/* Sprite/anim within this object */
	Uint32	flags;
#define MAPREF_SAVE	0x0001	/* Map dumps must record this reference */
#define MAPREF_SPRITE	0x0002	/* This is a sprite */
#define MAPREF_ANIM	0x0004	/* this is sequence of sprites */
#define MAPREF_WARP	0x0008	/* This is another map */
#define MAPREF_ANY	0xffff

	Uint32	frame;		/* Animation frame # */
	Uint32	fwait;		/* Animation delay counter */

	Sint32	xoffs, yoffs;	/* Incremented if > 0, decremented if < 0,
				   used for direction and soft scroll. */

	TAILQ_ENTRY(noderef) nrefs;	/* Node reference list */
};

TAILQ_HEAD(noderefs_head, noderef);

/* Coordinate within a map. */
struct node {
	struct	noderefs_head nrefsh;
	Uint32	nnrefs;
	Uint32	flags;
#define NODE_BLOCK	0x0000		/* Cannot walk through */
#define NODE_ORIGIN	0x0001		/* Origin of this map */
#define NODE_WALK	0x0002		/* Can walk through */
#define NODE_CLIMB	0x0004		/* Can climb (eg. ladder) */
#define NODE_SLIP	0x0008		/* Slippery */
#define NODE_BIO	0x0010		/* Cause Poison */
#define NODE_REGEN	0x0020		/* Cause HP Regeneration */
#define NODE_SLOW	0x0040		/* Decrease speed by v1 */
#define NODE_HASTE	0x0080		/* Increase speed by v1 */
#define NODE_ANIM	0x0100		/* Always animate node */
#define NODE_DONTSAVE	(NODE_ANIM)
	Uint32	v1, v2;			/* Extra properties */
	Uint32	nanims;			/* Animation count */
};

/* Region within the world. */
struct map {
	struct	object obj;
	struct	viewport *view;

	Uint32	flags;
#define MAP_FOCUSED	0x0001		/* Being displayed */
#define MAP_VARTILEGEO	0x0010		/* Variable tile geometry */
#define MAP_2D		0x0020		/* Two-dimensional */

	Uint32	redraw;			/* Redraw at next tick */
	Uint32	mapw, maph;		/* Map geometry */
	Uint32	tilew, tileh;		/* Tile geometry */
	Uint32	shtilex, shtiley;	/* Tile shift (optimization) */
	Uint32	defx, defy;		/* Map origin */
	struct	node **map;		/* Array of nodes */

	pthread_t	draw_th;	/* Map rendering thread */
	pthread_mutex_t lock;		/* Lock on all nodes */
	
	SLIST_ENTRY(map) wmaps;		/* Active maps */
};

struct map	*map_create(char *, char *, Uint32);
int		 map_load(void *, int);
int		 map_save(void *, int);
int		 map_destroy(void *);

int		 map_focus(struct map *);
int		 map_unfocus(struct map *);
void		 map_clean(struct map *, struct object *, Uint32, Uint32,
		     Uint32);
void		 map_allocnodes(struct map *, Uint32, Uint32, Uint32, Uint32);
void		 map_freenodes(struct map *);
void		 map_plot_sprite(struct map *, SDL_Surface *, Uint32, Uint32);
void		 map_dump(struct map *);

struct noderef	*node_addref(struct node *, void *, Uint32, Uint32);
struct noderef	*node_findref(struct node *, void *, Sint32, Uint32);
int		 node_delref(struct node *, struct noderef *);

struct noderef	*node_popref(struct node *);
int		 node_pushref(struct node *, struct noderef *);

