/*	$Csoft: map.h,v 1.51 2002/12/15 15:06:21 vedge Exp $	*/
/*	Public domain	*/

#define TILEW		32
#define TILEH		32
#define TILEW_SHIFT	5
#define TILEH_SHIFT	5

#include <engine/transform.h>

enum noderef_type {
	NODEREF_SPRITE,
	NODEREF_ANIM,
	NODEREF_WARP
};

/* Reference within a node. */
struct noderef {
#ifdef DEBUG
	char	magic[18];
#define NODEREF_MAGIC "agar map noderef"
#endif
	enum noderef_type	 type;	/* Type of reference */
	Uint32			 flags;
#define NODEREF_SAVEABLE	0x01	/* Saveable reference */
#define NODEREF_BLOCK		0x04	/* Block other objects on maps */
#define NODEREF_EPHEMERAL	0x00

	struct object	*pobj;		/* Object pointer */
	Uint32		 offs;		/* Sprite/anim array offset */

	Sint16	xcenter, ycenter;	/* Sprite/anim centering offsets */
	Sint16	xmotion, ymotion;	/* Sprite/anim motion offsets */

	union {
		struct {
			Uint32	flags;
#define NODEREF_ANIM_AUTO	0x01		/* Auto increment frame# */
			Uint32	frame;		/* Current frame# */
		} anim;
		struct {
			char	*map;		/* Map identifier */
			Uint32	 x, y;		/* Origin override */
			Uint8	 dir;		/* Default direction */
		} warp;
	} data;

	SLIST_HEAD(, transform) transforms;	/* Run-time tile transforms */
	TAILQ_ENTRY(noderef) nrefs;		/* Node reference list */
};

TAILQ_HEAD(noderefq, noderef);

/* Coordinate within a map. */
struct node {
#ifdef DEBUG
	char	magic[16];
#define NODE_MAGIC "agar map node"
	int	x, y;
#endif
	struct noderefq	 nrefs;		/* Items on this node */
	Uint32	 flags;
#define NODE_BLOCK	0x0001		/* Cannot walk through */
#define NODE_ORIGIN	0x0002		/* Origin of this map */
#define NODE_WALK	0x0004		/* Can walk through */
#define NODE_CLIMB	0x0008		/* Can climb (eg. ladder) */
#define NODE_SLIP	0x0010		/* Slippery */
#define NODE_BIO	0x0100		/* Cause Poison */
#define NODE_REGEN	0x0200		/* Cause HP Regeneration */
#define NODE_SLOW	0x0400		/* Decrease speed by v1 */
#define NODE_HASTE	0x0800		/* Increase speed by v1 */
#define NODE_EPHEMERAL	0x0000

	Uint32	 v1;			/* Extra property */
	Uint32	 nanims;		/* Animation count (optimization) */
};

enum map_type {
	MAP_2D,
	MAP_3D
};

/* Region within the world. */
struct map {
	struct object	 obj;
	enum map_type	 type;
	
	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
	
	int	  redraw;	/* Redraw at next tick. XXX */
	Uint32	  mapw, maph;	/* Map geometry */
	int	  tilew, tileh;	/* Tile geometry */
	Uint16	  zoom;		/* Zoom (%) */
	Uint32	  defx, defy;	/* Map origin */
	struct node	**map;	/* Array of nodes */
};

void		 map_init(struct map *, enum map_type, char *, char *);
int		 map_load(void *, int);
int		 map_save(void *, int);
void		 map_destroy(void *);
void		 map_alloc_nodes(struct map *, Uint32, Uint32);
void		 map_free_nodes(struct map *);
void		 map_shrink(struct map *, Uint32, Uint32);
void		 map_grow(struct map *, Uint32, Uint32);
void		 map_adjust(struct map *, Uint32, Uint32);
void		 map_set_zoom(struct map *, Uint16);
#ifdef DEBUG
void		 map_verify(struct map *);
#endif

void		 noderef_init(struct noderef *);
void		 noderef_destroy(struct noderef *);
void		 noderef_load(int, struct object_table *, struct node *,
		     struct noderef **);
void		 noderef_save(struct fobj_buf *, struct object_table *,
		     struct noderef *);

void		 node_init(struct node *, int, int);
void		 node_load(int, struct object_table *, struct node *);
void		 node_save(struct fobj_buf *, struct object_table *,
		     struct node *);
void		 node_destroy(struct node *, int, int);
void		 node_draw(struct map *, struct node *, Uint32, Uint32);

void		 node_move_ref(struct noderef *, struct node *, struct node *);
void		 node_copy_ref(struct noderef *, struct node *);
void		 node_remove_ref(struct node *, struct noderef *);
struct noderef	*node_add_sprite(struct node *, void *, Uint32);
struct noderef	*node_add_anim(struct node *, void *, Uint32, Uint32);
struct noderef	*node_add_warp(struct node *, char *, Uint32, Uint32, Uint8);

