/*	$Csoft: map.h,v 1.62 2003/02/22 11:42:38 vedge Exp $	*/
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
	char	magic[5];
#define NODEREF_MAGIC "nref"
#endif
	enum noderef_type	 type;		/* Type of reference */

	Uint32			 flags;
#define NODEREF_SAVEABLE	0x01		/* Saveable reference */
#define NODEREF_BLOCK		0x04		/* Similar to NODE_BLOCK */

	Uint8		 layer;			/* Layer# */
	struct object	*pobj;			/* Object pointer */
	Uint32		 offs;			/* Sprite/anim array offset */
	Sint16		 xcenter, ycenter;	/* Centering offsets */
	Sint16		 xmotion, ymotion;	/* Motion offsets */
	union {
		struct {
			Uint8	flags;
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
	char	magic[5];
#define NODE_MAGIC "node"
	Uint32	x, y;
#endif
	struct noderefq	 nrefs;		/* Items on this node */
	Uint32		 flags;
#define NODE_ORIGIN	0x00001		/* Origin of this map */
#define NODE_WALK	0x00002		/* Can walk through */
#define NODE_CLIMB	0x00004		/* Can climb (eg. ladder) */
#define NODE_SLIP	0x00008		/* Slippery */
#define NODE_BIO	0x00010		/* Cause Poison */
#define NODE_REGEN	0x00020		/* Cause HP Regeneration */
#define NODE_SLOW	0x00040		/* Decrease speed */
#define NODE_HASTE	0x00080		/* Increase speed */
#define NODE_EDGE_N	0x00100		/* Terrain edges (for map edition) */
#define NODE_EDGE_S	0x00200
#define NODE_EDGE_W	0x00400
#define NODE_EDGE_E	0x00800
#define NODE_EDGE_NW	0x01000
#define NODE_EDGE_NE	0x02000
#define NODE_EDGE_SW	0x04000
#define NODE_EDGE_SE	0x08000
#define NODE_EDGE_ANY	(NODE_EDGE_N|NODE_EDGE_S|NODE_EDGE_W|NODE_EDGE_E| \
			 NODE_EDGE_NW|NODE_EDGE_NE|NODE_EDGE_SW|NODE_EDGE_SE)
#define NODE_HAS_ANIM	0x10000		/* Contains an anim? (optimization) */
#define NODE_EPHEMERAL	NODE_HAS_ANIM
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
	
	Uint32		  mapw, maph;	/* Map geometry */
	Uint16		  zoom;		/* Zoom (%) */
	Sint16		  ssx, ssy;	/* Soft scrolling offsets */
	int		  tilew, tileh;	/* Tile geometry */
	Uint32		  defx, defy;	/* Map origin */
	struct node	**map;		/* Arrays of nodes */
	int		  redraw;	/* Redraw (for tile-based mode) */
#if defined(DEBUG) && defined(SERIALIZATION)
	pthread_t	  check_th;	/* Verify map integrity */
#endif
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

void		 noderef_init(struct noderef *);
void		 noderef_destroy(struct noderef *);
void		 noderef_load(int, struct object_table *, struct node *,
		     struct noderef **);
void		 noderef_save(struct fobj_buf *, struct object_table *,
		     struct noderef *);

void		 node_load(int, struct object_table *, struct node *);
void		 node_save(struct fobj_buf *, struct object_table *,
		     struct node *);
void		 node_destroy(struct node *);

void		 node_move_ref(struct noderef *, struct node *, struct node *);
void		 node_copy_ref(struct noderef *, struct node *);
void		 node_remove_ref(struct node *, struct noderef *);
void		 node_moveup_ref(struct node *, struct noderef *);
void		 node_movedown_ref(struct node *, struct noderef *);
void		 node_movetail_ref(struct node *, struct noderef *);
void		 node_movehead_ref(struct node *, struct noderef *);
struct noderef	*node_add_sprite(struct node *, void *, Uint32);
struct noderef	*node_add_anim(struct node *, void *, Uint32, Uint8);
struct noderef	*node_add_warp(struct node *, char *, Uint32, Uint32, Uint8);

extern __inline__ void	 node_init(struct node *, int, int);
extern __inline__ void	 noderef_draw(struct map *, struct noderef *,
			     Sint16, Sint16);

#ifdef DEBUG
extern int	 map_nodesigs;

# define MAP_CHECK_NODE(node, mx, my) do {			\
	if (map_nodesigs &&					\
	    (strncmp(NODE_MAGIC, (node)->magic, 4) != 0 ||	\
	     (node)->x != (mx) || (node)->y != (my))) {		\
		fatal("bad node\n");				\
	}							\
} while (0)
# define MAP_CHECK_NODEREF(nref) do {				\
	if (map_nodesigs &&					\
	    strncmp(NODEREF_MAGIC, (nref)->magic, 4) != 0) {	\
		fatal("bad nref\n");				\
	}							\
} while (0)
#else
# define MAP_CHECK_NODE(node, mx, my)
# define MAP_CHECK_NODEREF(nref, mx, my)
#endif /* DEBUG */

