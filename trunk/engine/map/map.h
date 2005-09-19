/*	$Csoft: map.h,v 1.18 2005/09/17 13:40:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define TILESZ	16			/* Default tile size in pixels */

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
	
	u_int flags;
#define NODEREF_BLOCK		0x001	/* Tile block */
#define NODEREF_CLIMBABLE	0x002	/* Surface is climbable */
#define NODEREF_SLIPPERY	0x004	/* Surface is slippery */
#define NODEREF_JUMPABLE	0x008	/* Element is jumpable */
#define NODEREF_NOSAVE		0x100	/* Non persistent */
#define NODEREF_MOUSEOVER	0x200	/* Mouse overlap (for editor) */
#define NODEREF_SELECTED	0x400	/* Selection (for editor) */

	Sint8 friction;			/* Coefficient of friction */
	Uint8 layer;			/* Associated layer */
	void *p;			/* User pointer (non-persistent) */

	struct {
		Sint16 xcenter, ycenter;	/* Centering offsets */
		Sint16 xmotion, ymotion;	/* Motion offsets */
		Sint16 xorigin, yorigin;	/* Origin point */
		SDL_Rect rs;			/* Source rectangle */
		Uint8 edge;			/* Edge type (for edition) */
	} r_gfx;
	union {
		struct {
			struct object *obj;	/* Gfx object */
			Uint32 offs;		/* Sprite index */
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
	int pixsz;				/* Scaled pixel size */
};

struct mapmod {
	enum {
		MAPMOD_NODECHG,
		MAPMOD_LAYERADD,
		MAPMOD_LAYERDEL
	} type;
	union {
		struct {
			struct node node;
			int x, y;
		} nodechg;
		struct {
			int nlayer;
		} layermod;
	} data;
#define mm_nodechg data.nodechg
#define mm_layeradd data.layermod
#define mm_layerdel data.layermod
};

struct mapmod_blk {
	struct mapmod *mods;
	u_int nmods;
	u_char cancel;
};

struct map {
	struct space space;

	pthread_mutex_t lock;
	u_int flags;
#define MAP_SAVE_CAM0POS	0x01	/* Save the camera 0 position */
#define MAP_SAVE_CAM0ZOOM	0x02	/* Save the camera 0 zoom factor */
#define MAP_SAVED_FLAGS		(MAP_SAVE_CAM0POS|MAP_SAVE_CAM0ZOOM)

	u_int mapw, maph;		/* Map geometry */
	int cur_layer;			/* Layer being edited */
	struct {
		int x, y;		/* Origin coordinates */
		int layer;		/* Default sprite layer */
	} origin;
	struct node **map;		/* Arrays of nodes */
	int redraw;			/* Redraw (for tile-based mode) */

	struct map_layer *layers;	/* Layer descriptions */
	u_int nlayers;

	struct map_camera *cameras;	/* Views */
	u_int ncameras;

	struct mapmod_blk *blks;	/* Saved modifications */
	u_int nblks;
	u_int curblk;
	u_int nmods;
};

__BEGIN_DECLS
struct map	*map_new(void *, const char *);
void		 map_init(void *, const char *);
void		 map_reinit(void *);
int		 map_load(void *, struct netbuf *);
int		 map_save(void *, struct netbuf *);
void		 map_destroy(void *);
void		*map_edit(void *);

int	 map_alloc_nodes(struct map *, u_int, u_int);
void	 map_free_nodes(struct map *);
int	 map_resize(struct map *, u_int, u_int);
void	 map_set_zoom(struct map *, int, u_int);
int	 map_push_layer(struct map *, const char *);
void	 map_pop_layer(struct map *);
void	 map_init_layer(struct map_layer *, const char *);
void	 map_init_camera(struct map_camera *, const char *);
void	 map_init_modblks(struct map *);
int	 map_add_camera(struct map *, const char *);

void	 mapmod_begin(struct map *);
void	 mapmod_end(struct map *);
void	 mapmod_cancel(struct map *);
void	 mapmod_nodechg(struct map *, int, int);
void	 mapmod_layeradd(struct map *, int);
void	 map_undo(struct map *);
void	 map_redo(struct map *);

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
void		 noderef_attr_color(u_int, int, Uint8 *);

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
