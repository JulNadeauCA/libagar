/*	$Csoft: map.h,v 1.88 2003/04/24 06:58:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define TILEW		32
#define TILEH		32
#define TILEW_SHIFT	5
#define TILEH_SHIFT	5

#define MAP_MIN_WIDTH		4
#define MAP_MIN_HEIGHT		4
#define MAP_MAX_WIDTH		32767
#define MAP_MAX_HEIGHT		32767
#define MAP_MAX_LAYERS		256
#define NODE_MAX_NODEREFS	32767
#define NODEREF_MAX_TRANSFORMS	16384
#define NODEREF_MAX_CENTER	65535
#define NODEREF_MAX_MOTION	65535
#define MAP_LAYER_NAME_MAX	128

#include <engine/transform.h>

#include "begin_code.h"

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
	enum noderef_type type;			/* Type of reference */
	Uint8		  flags;
#define NODEREF_SAVEABLE	0x01		/* Saveable reference */
#define NODEREF_BLOCK		0x04		/* Similar to NODE_BLOCK */

	Uint8		 layer;			/* Layer# */
	struct object	*pobj;			/* Object pointer */
	Uint32		 offs;			/* Sprite/anim array offset */
	Sint16		 xcenter, ycenter;	/* Gfx centering displacement */
	Sint16		 xmotion, ymotion;	/* Gfx motion displacement */
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

	pthread_mutex_t	  lock;
	unsigned int	  mapw, maph;	/* Map geometry */
	Uint16		  zoom;		/* Zoom (%) */
	Sint16		  ssx, ssy;	/* Soft scrolling offsets */
	unsigned int	  tilew, tileh;	/* Tile geometry */
	int		  cur_layer;	/* Layer being edited */
	
	struct {
		int	  x, y;		/* Origin coordinates */
		int	  layer;	/* Default sprite layer */
	} origin;

	struct node	**map;		/* Arrays of nodes */
	int		  redraw;	/* Redraw (for tile-based mode) */

	struct map_layer *layers;	/* Layer descriptions */
	Uint32		 nlayers;
#if defined(DEBUG) && defined(THREADS)
	pthread_t	  check_th;	/* Verify map integrity */
#endif
};

__BEGIN_DECLS
extern DECLSPEC void	 map_init(struct map *, char *);
extern DECLSPEC int	 map_load(void *, struct netbuf *);
extern DECLSPEC int	 map_save(void *, struct netbuf *);
extern DECLSPEC void	 map_destroy(void *);
extern DECLSPEC int	 map_alloc_nodes(struct map *, unsigned int,
			                 unsigned int);
extern DECLSPEC void	 map_free_nodes(struct map *);
extern DECLSPEC int	 map_resize(struct map *, unsigned int, unsigned int);
extern DECLSPEC void	 map_set_zoom(struct map *, Uint16);
extern DECLSPEC int	 map_push_layer(struct map *, char *);
extern DECLSPEC void	 map_pop_layer(struct map *);
extern DECLSPEC void	 noderef_init(struct noderef *);
extern DECLSPEC int	 noderef_set_center(struct noderef *, int, int);
extern DECLSPEC int	 noderef_set_motion(struct noderef *, int, int);
extern DECLSPEC void	 noderef_destroy(struct noderef *);
extern DECLSPEC int	 noderef_load(struct netbuf *, struct object_table *,
			              struct node *, struct noderef **);
extern DECLSPEC void	 noderef_save(struct netbuf *, struct object_table *,
			              struct noderef *);
extern __inline__ void	 noderef_draw(struct map *, struct noderef *, int, int);

extern DECLSPEC void	 node_init(struct node *);
extern DECLSPEC int	 node_load(struct netbuf *, struct object_table *,
			           struct node *);
extern DECLSPEC void	 node_save(struct netbuf *, struct object_table *,
			           struct node *);
extern DECLSPEC void	 node_destroy(struct node *);
extern DECLSPEC void	 node_clear_layer(struct node *, Uint8);

extern DECLSPEC int	 node_move_ref(struct noderef *, struct node *,
			               struct map *, int, int);
extern DECLSPEC void	 node_move_ref_direct(struct noderef *, struct node *,
			                      struct node *);

extern DECLSPEC void	 node_moveup_ref(struct node *, struct noderef *);
extern DECLSPEC void	 node_movedown_ref(struct node *, struct noderef *);
extern DECLSPEC void	 node_movetail_ref(struct node *, struct noderef *);
extern DECLSPEC void	 node_movehead_ref(struct node *, struct noderef *);
extern DECLSPEC void	 node_remove_ref(struct node *, struct noderef *);

extern DECLSPEC struct noderef	*node_copy_ref(struct noderef *, struct node *);
extern DECLSPEC struct noderef	*node_add_sprite(struct node *, void *, Uint32);
extern DECLSPEC struct noderef	*node_add_anim(struct node *, void *, Uint32,
				               Uint8);
extern DECLSPEC struct noderef	*node_add_warp(struct node *, char *, int, int,
				               Uint8);
__END_DECLS

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

#include "close_code.h"
#endif /* _AGAR_MAP_H_ */
