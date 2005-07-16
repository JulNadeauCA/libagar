/*	$Csoft: map.h,v 1.8 2005/06/18 16:37:19 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define TILESZ	32			/* Default tile size in pixels */

#define MAP_MIN_TILESZ		7
#define MAP_MAX_TILESZ		16384	/* For soft-scrolling */
#define MAP_MAX_WIDTH		32767
#define MAP_MAX_HEIGHT		32767
#define MAP_MAX_LAYERS		256
#define MAP_MAX_CAMERAS		256
#define MAP_LAYER_NAME_MAX	128
#define MAP_CAMERA_NAME_MAX	128
#define NODE_MAX_NODEREFS	32767
#define NODEREF_MAX_TRANSFORMS	16384
#define NODEREF_MAX_MASKS	16384

#include <engine/space/space.h>
#include <engine/map/transform.h>
#include <engine/map/nodemask.h>
#include <engine/gobject.h>

#include "begin_code.h"

enum noderef_type {
	NODEREF_SPRITE,			/* Reference to a sprite */
	NODEREF_ANIM,			/* Reference to an animation */
	NODEREF_WARP,			/* Reference to another location */
	NODEREF_GOBJ			/* Reference to a geometric object */
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
	
	Uint8 flags;
#define NODEREF_WALK		0x01		/* Surface is walkable */
#define NODEREF_CLIMB		0x02		/* Surface is climbable */
#define NODEREF_SLIP		0x04		/* Surface is slippery */
#define NODEREF_BIO		0x08		/* Contact induces Poison */
#define NODEREF_REGEN		0x10		/* Contact induces Regen */
#define NODEREF_NOSAVE		0x20		/* Non persistent */
#define NODEREF_MOUSEOVER	0x40		/* Mouse overlap (for editor) */
#define NODEREF_SELECTED	0x80		/* Selection (for editor) */

	Sint8 friction;		/* Coefficient of acceleration or friction */
	Uint8 layer;		/* Associated layer */

	struct {
		Sint16 xcenter, ycenter;	/* Centering offsets */
		Sint16 xmotion, ymotion;	/* Motion offsets */
		Sint16 xorigin, yorigin;	/* Origin point */
		Uint8 edge;			/* Edge type (for edition) */
	} r_gfx;
	union {
		struct {
			struct object *obj;	/* Gfx object */
			Uint32 offs;		/* Sprite index */
			SDL_Rect rs;		/* Source rectangle */
		} sprite;
		struct {
			struct object *obj;	/* Gfx object */
			Uint32 offs;		/* Anim index */
		} anim;
		struct {
			char *map;		/* Map identifier */
			int x, y;		/* Origin override */
			Uint8 dir;		/* Default direction */
		} warp;
		struct {
			void *p;		/* Geometric object */
			Uint32 flags;
		} gobj;
	} nref;
	struct transformq transforms;		/* Transformations to apply */
	struct nodemaskq masks;			/* Collision detection masks */
	TAILQ_ENTRY(noderef) nrefs;		/* Node's reference stack */
#define r_sprite	nref.sprite
#define r_anim		nref.anim
#define r_warp		nref.warp
#define r_gobj		nref.gobj
};

TAILQ_HEAD(noderefq, noderef);

struct node {
	struct noderefq	 nrefs;			/* Items on this node */
};

struct map_layer {
	char name[MAP_LAYER_NAME_MAX];
	int visible;				/* Show/hide flag */
	Sint16 xinc, yinc;			/* Rendering direction */
	Uint8 alpha;				/* Transparency value */
};

enum map_camera_alignment {
	MAP_UPPER_LEFT,
	MAP_MIDDLE_LEFT,
	MAP_LOWER_LEFT,
	MAP_UPPER_RIGHT,
	MAP_MIDDLE_RIGHT,
	MAP_LOWER_RIGHT,
	MAP_CENTER,
	MAP_LOWER_CENTER,
	MAP_UPPER_CENTER
};

struct map_camera {
	char name[MAP_CAMERA_NAME_MAX];
	int flags;
	int x, y;				/* Position of camera */
	enum map_camera_alignment alignment;	/* View alignment */
	u_int zoom;				/* Zoom (%) */
	int tilesz;				/* Tile size */
};

struct map {
	struct space space;

	pthread_mutex_t lock;
	u_int mapw, maph;		/* Map geometry */
	int cur_layer;			/* Layer being edited */
	struct {
		int x, y;		/* Origin coordinates */
		int layer;		/* Default sprite layer */
	} origin;
	struct node **map;		/* Arrays of nodes */
	int redraw;			/* Redraw (for tile-based mode) */

	struct map_layer *layers;	/* Layer descriptions */
	u_int		 nlayers;

	struct map_camera *cameras;	/* Views */
	u_int		  ncameras;
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

int	 map_alloc_nodes(struct map *, u_int, u_int);
void	 map_free_nodes(struct map *);
int	 map_resize(struct map *, u_int, u_int);
void	 map_set_zoom(struct map *, int, u_int);
int	 map_push_layer(struct map *, const char *);
void	 map_pop_layer(struct map *);
void	 map_init_layer(struct map_layer *, const char *);
void	 map_init_camera(struct map_camera *, const char *);
int	 map_add_camera(struct map *, const char *);

void		 noderef_init(struct noderef *, enum noderef_type);
__inline__ void	 noderef_set_center(struct noderef *, int, int);
__inline__ void	 noderef_set_motion(struct noderef *, int, int);
__inline__ void	 noderef_set_friction(struct noderef *, int);
__inline__ void	 noderef_set_layer(struct noderef *, int);

void	 	 noderef_destroy(struct map *, struct noderef *);
int		 noderef_load(struct map *, struct netbuf *, struct node *,
			      struct noderef **);
void	 	 noderef_save(struct map *, struct netbuf *, struct noderef *);
__inline__ int	 noderef_extent(struct map *, struct noderef *, SDL_Rect *,
		                int);
__inline__ void	 noderef_draw(struct map *, struct noderef *, int, int, int);
__inline__ void	 noderef_set_sprite(struct noderef *, struct map *, void *,
		                    Uint32);
__inline__ void	 noderef_set_anim(struct noderef *, struct map *, void *,
		                  Uint32);
struct noderef  *noderef_locate(struct map *, int, int, int);

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
void		 node_swap_layers(struct map *, struct node *, int, int);

struct noderef	*node_add_sprite(struct map *, struct node *, void *, Uint32);
struct noderef	*node_add_anim(struct map *, struct node *, void *, Uint32);
struct noderef	*node_add_warp(struct map *, struct node *, const char *, int,
		               int, Uint8);
struct noderef	*node_add_gobj(struct map *, struct node *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_H_ */
