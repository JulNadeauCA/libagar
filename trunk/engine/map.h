/*	$Csoft: map.h,v 1.48 2002/11/30 02:09:47 vedge Exp $	*/
/*	Public domain	*/

#define TILEW		32
#define TILEH		32
#define TILEW_SHIFT	5
#define TILEH_SHIFT	5

#ifndef CHARW
#define CHARW	32
#endif
#ifndef CHARH
#define CHARH	64
#endif

enum {
	MAP_MINWIDTH	= 10,
	MAP_MINHEIGHT	= 10
};

/* XXX use unions */
struct noderef {
#ifdef DEBUG
	char	magic[18];
#define NODEREF_MAGIC "agar map noderef"
#endif
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
#define MAPREF_DONTSAVE		0
	Uint32	offs;
	
	Sint32	xoffs;		/* X motion offset */
	Sint32	yoffs;		/* Y motion offset */

	/* Animation variables */
	int	frame;		/* Animation frame # */
	int	fdelta;		/* For MAPREF_ANIM_INDEPENDENT */

	TAILQ_ENTRY(noderef) nrefs;	/* Node reference list */
};

TAILQ_HEAD(noderefq, noderef);

/* Coordinate within a map. */
struct node {
#ifdef DEBUG
	char	magic[16];
#define NODE_MAGIC "agar map node"
	int	x, y;
#endif
	struct	noderefq nrefsh;
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
#define NODE_DONTSAVE	0x0000

	Uint32	v1, v2;			/* Extra properties */
	Uint32	nanims;			/* Animation count */
};

/* Region within the world. */
struct map {
	struct object	  obj;

	Uint32		  flags;
#define MAP_RLE_COMPRESSION	0x01	/* RLE-compress the nodes */
#define MAP_2D			0x20	/* Two-dimensional */

	int		  redraw;	/* Redraw at next tick. XXX */
	Uint32		  fps;		/* Maximum fps */
	Uint32		  mapw, maph;	/* Map geometry */
	int		  tilew, tileh;	/* Tile geometry */
	Uint32		  defx, defy;	/* Map origin */
	struct node	**map;		/* Array of nodes */
	int		  zoom;		/* Zoom (%) */

	pthread_t	  draw_th;	/* Map rendering thread */
	pthread_mutex_t	  lock;		/* Recursive lock on all nodes, and all
					   references inside them */
	pthread_mutexattr_t lockattr;
};

void	map_init(struct map *, char *, char *, Uint32);
int	map_load(void *, int);
int	map_save(void *, int);
void	map_destroy(void *);

void	map_clean(struct map *, struct object *, Uint32, Uint32, Uint32);
void	map_allocnodes(struct map *, Uint32, Uint32);
void	map_freenodes(struct map *);
void	map_shrink(struct map *, Uint32, Uint32);
void	map_grow(struct map *, Uint32, Uint32);
void	map_adjust(struct map *, Uint32, Uint32);
#ifdef DEBUG
void	map_verify(struct map *);
#endif
void	map_set_zoom(struct map *, int);

struct noderef	*node_addref(struct node *, void *, Uint32, Uint32);
struct noderef	*node_findref(struct map *, struct node *, void *, Sint32,
		     Uint32);
int		 node_delref(struct node *, struct noderef *);
void		 node_draw(struct map *, struct node *, Uint32, Uint32);

