/*	$Csoft: map.h,v 1.108 2004/04/10 02:35:50 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define TILESZ		32
#define TILESHIFT	5

#define MAP_MIN_TILESZ		7
#define MAP_MAX_TILESZ		16384	/* For soft-scrolling */
#define MAP_MAX_WIDTH		32767
#define MAP_MAX_HEIGHT		32767
#define MAP_MAX_LAYERS		256
#define MAP_LAYER_NAME_MAX	128
#define NODE_MAX_NODEREFS	32767
#define NODEREF_MAX_CENTER	32767
#define NODEREF_MAX_MOTION	32767
#define NODEREF_MAX_TRANSFORMS	16384
#define NODEREF_MAX_MASKS	16384

#include <engine/transform.h>
#include <engine/nodemask.h>

#include "begin_code.h"

enum noderef_type {
	NODEREF_SPRITE,			/* Reference to a sprite */
	NODEREF_ANIM,			/* Reference to an animation */
	NODEREF_WARP			/* Reference to another map */
};

enum noderef_edge {
	NODEREF_EDGE_NONE,
	NODEREF_EDGE_NW,
	NODEREF_EDGE_N,
	NODEREF_EDGE_NE,
	NODEREF_EDGE_W,
	NODEREF_EDGE_FILL,
	NODEREF_EDGE_E,
	NODEREF_EDGE_SW,
	NODEREF_EDGE_S,
	NODEREF_EDGE_SE
};

struct noderef {
	enum noderef_type type;		/* Type of element */
	Uint8	flags;
#define NODEREF_WALK	0x01		/* Surface is walkable */
#define NODEREF_CLIMB	0x02		/* Surface is climbable */
#define NODEREF_SLIP	0x04		/* Surface is slippery */
#define NODEREF_BIO	0x08		/* Contact induces Poison */
#define NODEREF_REGEN	0x10		/* Contact induces Regen */
#define NODEREF_NOSAVE	0x20		/* Non persistent */
#define NODEREF_NOSCALE	0x40		/* Don't scale pixmaps (ie. vg) */

	Sint8	friction;		/* Coefficient of friction (if n>0),
					   or acceleration (if n<0) */
	Uint8	layer;			/* Associated layer# */
	struct {
		Sint16	xcenter, ycenter;	/* Centering offsets */
		Sint16	xmotion, ymotion;	/* Motion offsets */
		Uint8	edge;			/* Edge type (for edition) */
	} r_gfx;
	union {
		struct {
			struct object	*obj;		/* Gfx object */
			Uint32		 offs;		/* Sprite index */
		} sprite;
		struct {
			struct object	*obj;		/* Gfx object */
			Uint32		 offs;		/* Anim index */

			Uint8	flags;
#define NODEREF_SHD_FRAME 0x01			/* Use shared frame# */
#define NODEREF_PVT_FRAME 0x02			/* Use private frame# */
			Uint32	frame;		/* Private frame# */
		} anim;
		struct {
			char	*map;		/* Map identifier */
			int	 x, y;		/* Origin override */
			Uint8	 dir;		/* Default direction */
		} warp;
	} nref;
	struct transformq transforms;		/* Transformations to apply */
	struct nodemaskq masks;			/* Collision detection masks */
	TAILQ_ENTRY(noderef) nrefs;		/* Node's reference stack */
#define r_sprite	nref.sprite
#define r_anim		nref.anim
#define r_warp		nref.warp
};

TAILQ_HEAD(noderefq, noderef);

struct node {
	struct noderefq	 nrefs;			/* Items on this node */
};

struct map_layer {
	char	 name[MAP_LAYER_NAME_MAX];	/* Identifier */
	int	 visible;			/* Layer is visible? */
	Sint16	 xinc, yinc;			/* Rendering direction */
	Uint8	 alpha;				/* Transparency */
};

struct map {
	struct object	  obj;

	pthread_mutex_t	  lock;
	unsigned int	  mapw, maph;	/* Map geometry */
	Uint16		  zoom;		/* Zoom (%) */
	Sint16		  ssx, ssy;	/* Soft scrolling offsets */
	unsigned int	  tilesz;	/* Tile size */
	int		  cur_layer;	/* Layer being edited */
	
	struct {
		int	  x, y;		/* Origin coordinates */
		int	  layer;	/* Default sprite layer */
	} origin;

	struct node	**map;		/* Arrays of nodes */
	int		  redraw;	/* Redraw (for tile-based mode) */

	struct map_layer *layers;	/* Layer descriptions */
	Uint32		 nlayers;
};

__BEGIN_DECLS
struct map	*map_new(void *, const char *);
void		 map_init(void *, const char *);
void		 map_reinit(void *);
int		 map_load(void *, struct netbuf *);
int		 map_save(void *, struct netbuf *);
void		 map_destroy(void *);

#ifdef EDITION
struct window	*map_edit(void *);
#endif

int		 map_alloc_nodes(struct map *, unsigned int, unsigned int);
void		 map_free_nodes(struct map *);
int		 map_resize(struct map *, unsigned int, unsigned int);
void		 map_set_zoom(struct map *, Uint16);
int		 map_push_layer(struct map *, const char *);
void		 map_pop_layer(struct map *);
void		 map_init_layer(struct map_layer *, const char *);

void		 noderef_init(struct noderef *, enum noderef_type);
__inline__ void	 noderef_set_center(struct noderef *, int, int);
__inline__ void	 noderef_set_motion(struct noderef *, int, int);
__inline__ void	 noderef_set_friction(struct noderef *, int);
void	 	 noderef_destroy(struct map *, struct noderef *);
int		 noderef_load(struct map *, struct netbuf *, struct node *,
			      struct noderef **);
void	 	 noderef_save(struct map *, struct netbuf *, struct noderef *);
__inline__ void	 noderef_draw(struct map *, struct noderef *, int, int);

__inline__ void	 node_init(struct node *);
int		 node_load(struct map *, struct netbuf *, struct node *);
void		 node_save(struct map *, struct netbuf *, struct node *);
void		 node_destroy(struct map *, struct node *);
void		 node_clear(struct map *, struct node *, int);

__inline__ void	 node_copy(struct map *, struct node *, int,
		           struct map *, struct node *, int);
void		 node_move_ref(struct map *, struct node *, struct noderef *,
		               struct map *, struct node *, int);
struct noderef	*node_copy_ref(const struct noderef *, struct map *,
		               struct node *, int);
void		 node_moveup_ref(struct node *, struct noderef *);
void		 node_movedown_ref(struct node *, struct noderef *);
void		 node_movetail_ref(struct node *, struct noderef *);
void		 node_movehead_ref(struct node *, struct noderef *);
void		 node_remove_ref(struct map *, struct node *, struct noderef *);

struct noderef	*node_add_sprite(struct map *, struct node *, void *, Uint32);
struct noderef	*node_add_anim(struct map *, struct node *, void *, Uint32,
		               Uint8);
struct noderef	*node_add_warp(struct map *, struct node *, const char *, int,
		               int, Uint8);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_H_ */
