/*	$Csoft: map.h,v 1.82 2003/03/25 13:42:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define TILEW		32
#define TILEH		32
#define TILEW_SHIFT	5
#define TILEH_SHIFT	5

#define MAP_MAX_WIDTH		32767
#define MAP_MAX_HEIGHT		32767
#define MAP_MAX_LAYERS		256
#define NODE_MAX_NODEREFS	32767
#define NODEREF_MAX_TRANSFORMS	16384
#define NODEREF_MAX_CENTER	65535
#define MAP_LAYER_NAME_MAX	128

#include <engine/transform.h>

enum noderef_type {
	NODEREF_SPRITE,
	NODEREF_ANIM,
	NODEREF_WARP
};

struct noderef {
#ifdef DEBUG
	int	magic;
#define NODEREF_MAGIC 0x1ad
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
			int	 x, y;		/* Origin override */
			Uint8	 dir;		/* Default direction */
		} warp;
	} data;
	SLIST_HEAD(, transform) transforms;	/* Run-time tile transforms */
	TAILQ_ENTRY(noderef) nrefs;		/* Node reference list */
};

TAILQ_HEAD(noderefq, noderef);

struct node {
#ifdef DEBUG
	int	magic;
#define NODE_MAGIC 0x60cd
#endif
	struct noderefq	 nrefs;		/* Items on this node */
	Uint32		 flags;
#define NODE_ORIGIN	0x00001		/* Origin (unused) */
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
#define NODE_EPHEMERAL	(NODE_ORIGIN)
};

struct map_layer {
	char	*name;		/* Identifier */
	int	 visible;	/* Layer is visible? */
	Sint16	 xinc, yinc;	/* X/Y increments for rendering */
	Uint8	 alpha;		/* Transparency */
};

struct map {
	struct object	  obj;

	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;

	unsigned int	  mapw, maph;	/* Map geometry */
	Uint16		  zoom;		/* Zoom (%) */
	Sint16		  ssx, ssy;	/* Soft scrolling offsets */
	unsigned int	  tilew, tileh;	/* Tile geometry */
	int		  defx, defy;	/* Map origin */
	int		  cur_layer;	/* Layer being edited */

	struct node	**map;		/* Arrays of nodes */
	int		  redraw;	/* Redraw (for tile-based mode) */

	struct map_layer *layers;	/* Layer descriptions */
	Uint32		 nlayers;

#if defined(DEBUG) && defined(THREADS)
	pthread_t	  check_th;	/* Verify map integrity */
#endif
};

void		 map_init(struct map *, char *, char *);
int		 map_load(void *, int);
int		 map_save(void *, int);
void		 map_destroy(void *);
int		 map_alloc_nodes(struct map *, unsigned int, unsigned int);
void		 map_free_nodes(struct map *);
int		 map_resize(struct map *, unsigned int, unsigned int);
void		 map_set_zoom(struct map *, Uint16);
int		 map_push_layer(struct map *, char *);
void		 map_pop_layer(struct map *);

void		 noderef_init(struct noderef *);
void		 noderef_destroy(struct noderef *);
int		 noderef_load(int, struct object_table *, struct node *,
		     struct noderef **);
void		 noderef_save(struct fobj_buf *, struct object_table *,
		     struct noderef *);
__inline__ void	 noderef_draw(struct map *, struct noderef *, int, int);

void		 node_init(struct node *);
int		 node_load(int, struct object_table *, struct node *);
void		 node_save(struct fobj_buf *, struct object_table *,
		     struct node *);
void		 node_destroy(struct node *);
void		 node_clear_layer(struct node *, Uint8);

void		 node_move_ref(struct noderef *, struct node *, struct node *);
struct noderef	*node_copy_ref(struct noderef *, struct node *);
void		 node_remove_ref(struct node *, struct noderef *);
void		 node_moveup_ref(struct node *, struct noderef *);
void		 node_movedown_ref(struct node *, struct noderef *);
void		 node_movetail_ref(struct node *, struct noderef *);
void		 node_movehead_ref(struct node *, struct noderef *);
struct noderef	*node_add_sprite(struct node *, void *, Uint32);
struct noderef	*node_add_anim(struct node *, void *, Uint32, Uint8);
struct noderef	*node_add_warp(struct node *, char *, int, int, Uint8);

#ifdef DEBUG
extern int map_nodesigs;
# define MAP_CHECK_NODE(node)					\
	if (map_nodesigs && (node)->magic != NODE_MAGIC)	\
		fatal("bad node");
# define MAP_CHECK_NODEREF(nref)				\
	if (map_nodesigs && (nref)->magic != NODEREF_MAGIC)	\
		fatal("bad nref");
#else
# define MAP_CHECK_NODE(node)
# define MAP_CHECK_NODEREF(nref)
#endif /* DEBUG */

#endif /* _AGAR_MAP_H_ */
