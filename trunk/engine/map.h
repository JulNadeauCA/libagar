/*	$Csoft: map.h,v 1.34 2002/05/13 06:51:13 vedge Exp $	*/

#ifndef TILEW
#define TILEW	32
#endif
#ifndef TILEH
#define TILEH	32
#endif

enum {
	MAP_MINWIDTH	= 10,
	MAP_MINHEIGHT	= 10
};

struct noderef {
	struct	object *pobj;	/* Object pointer */
	Uint32	flags;
#define MAPREF_SAVE		0x0001	/* Saveable reference */
#define MAPREF_SPRITE		0x0002	/* Single surface */
#define MAPREF_ANIM		0x0010	/* Sequence of frames */
#define MAPREF_ANIM_DELTA	0x0020	/* Increment per anim */
#define MAPREF_ANIM_INDEPENDENT	0x0040	/* Increment per map reference */
#define MAPREF_ANIM_STATIC	0x0080	/* Increment manually */
#define MAPREF_MAP_WARP		0x1000	/* Jump to another map */
#define MAPREF_MAP_NODE		0x2000	/* Node of another map */
#define MAPREF_ANY		0xffff
	Uint32	offs;
	Sint32	xoffs, yoffs;	/* Incremented if > 0, decremented if < 0,
				   used for direction and soft scroll. */

	int	frame, fdelta;	/* For MAPREF_ANIM_INDEPENDENT */

	TAILQ_ENTRY(noderef) nrefs;	/* Node reference list */
};

TAILQ_HEAD(nrefs_head, noderef);

/* Coordinate within a map. */
struct node {
	struct	nrefs_head nrefsh;
	Uint32	nnrefs;
	Uint32	flags;
#define NODE_BLOCK	0x0001		/* Cannot walk through */
#define NODE_ORIGIN	0x0002		/* Origin of this map */
#define NODE_WALK	0x0004		/* Can walk through */
#define NODE_CLIMB	0x0008		/* Can climb (eg. ladder) */
#define NODE_SLIP	0x0010		/* Slippery */
#define NODE_BIO	0x0100		/* Cause Poison */
#define NODE_REGEN	0x0200		/* Cause HP Regeneration */
#define NODE_SLOW	0x0400		/* Decrease speed by v1 */
#define NODE_HASTE	0x0800		/* Increase speed by v1 */
#define NODE_ANIM	0x1000		/* Always animate node */
#define NODE_OVERLAP	0x2000		/* Moving sprite covers node */
#define NODE_DONTSAVE	(NODE_ANIM|NODE_OVERLAP)
	Uint32	v1, v2;			/* Extra properties */
	Uint32	nanims;			/* Animation count */
};

/* Region within the world. */
struct map {
	struct	object obj;
	struct	viewport *view;

	Uint32	flags;
#define MAP_FOCUSED	0x0001		/* Being displayed */
#define MAP_2D		0x0020		/* Two-dimensional */

	int	redraw;			/* Redraw at next tick
					   (can be inconsistent) */
	Uint32	fps;			/* Maximum fps */
	Uint32	mapw, maph;		/* Map geometry */
	int	shtilex, shtiley;	/* Tile shift (optimization) */
	Uint32	defx, defy;		/* Map origin */
	struct	node **map;		/* Array of nodes */

	pthread_t	draw_th;	/* Map rendering thread */
	pthread_mutex_t lock;		/* Lock on all nodes, and all
					   references inside them */
	
	SLIST_ENTRY(map) wmaps;		/* Active maps */
};

#define MAP_COORD(x, view)	((x) / TILEW)

void	map_init(struct map *, char *, char *, Uint32);
int	map_load(void *, int);
int	map_save(void *, int);
void	map_destroy(void *);

void	map_draw(struct map *);
void	map_animate(struct map *);

void	map_focus(struct map *);
void	map_unfocus(struct map *);
void	map_clean(struct map *, struct object *, Uint32, Uint32, Uint32);
void	map_allocnodes(struct map *, Uint32, Uint32);
void	map_freenodes(struct map *);
#ifdef DEBUG
void	map_verify(struct map *);
#endif

struct noderef	*node_addref(struct node *, void *, Uint32, Uint32);
struct noderef	*node_findref(struct node *, void *, Sint32, Uint32);
int		 node_delref(struct node *, struct noderef *);
struct noderef	*node_popref(struct node *);
void		 node_pushref(struct node *, struct noderef *);

