/*	$Csoft: map.h,v 1.20 2005/09/20 13:46:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define AGTILESZ 16			/* Default tile size in pixels */

#define AG_MAX_TILESZ		16384
#define AG_MAP_MAXWIDTH		32767
#define AG_MAP_MAXHEIGHT	32767
#define AG_MAP_MAXLAYERS	256
#define AG_MAP_MAXCAMERAS	256
#define AG_MAP_MAXLAYERNAME	128
#define AG_MAP_MAXCAMERANAME	128
#define AG_NODE_MAXITEMS	32767
#define AG_NITEM_MAXTRANSFORMS 16384
#define AG_NITEM_MAXMASKS	16384

#include <engine/space/space.h>
#include <engine/map/transform.h>
#include <engine/map/nodemask.h>

#include "begin_code.h"

enum ag_nitem_type {
	AG_NITEM_SPRITE,		/* Reference to a sprite */
	AG_NITEM_ANIM,		/* Reference to an animation */
	AG_NITEM_WARP		/* Reference to another location */
};

typedef struct ag_nitem {
	enum ag_nitem_type type;	/* Type of element */
	
	u_int flags;
#define AG_NITEM_BLOCK	0x001	/* Tile block */
#define AG_NITEM_CLIMBABLE	0x002	/* Surface is climbable */
#define AG_NITEM_SLIPPERY	0x004	/* Surface is slippery */
#define AG_NITEM_JUMPABLE	0x008	/* Element is jumpable */
#define AG_NITEM_NOSAVE	0x100	/* Non persistent */
#define AG_NITEM_MOUSEOVER	0x200	/* Mouse overlap (for editor) */
#define AG_NITEM_SELECTED	0x400	/* Selection (for editor) */

	Sint8 friction;			/* Coefficient of friction */
	Uint8 layer;			/* Associated layer */
	void *p;			/* User pointer (non-persistent) */

	struct {
		Sint16 xcenter, ycenter;	/* Centering offsets */
		Sint16 xmotion, ymotion;	/* Motion offsets */
		Sint16 xorigin, yorigin;	/* Origin point */
		SDL_Rect rs;			/* Source rectangle */
	} r_gfx;
	union {
		struct {
			AG_Object *obj;		/* Gfx object */
			Uint32 offs;		/* Sprite index */
		} sprite;
		struct {
			AG_Object *obj;		/* Gfx object */
			Uint32 offs;		/* Anim index */
		} anim;
		struct {
			char *map;		/* Map identifier */
			int x, y;		/* Origin override */
			Uint8 dir;		/* Default direction */
		} warp;
	} nref;
	struct ag_transformq transforms;	/* Transformations to apply */
	struct ag_nodemaskq masks;		/* Collision detection masks */
	TAILQ_ENTRY(ag_nitem) nrefs;		/* Node's reference stack */
#define r_sprite	nref.sprite
#define r_anim		nref.anim
#define r_warp		nref.warp
} AG_Nitem;

TAILQ_HEAD(ag_nitemq, ag_nitem);

typedef struct ag_node {
	struct ag_nitemq nrefs;		/* Items on this node */
} AG_Node;

typedef struct ag_map_layer {
	char name[AG_MAP_MAXLAYERNAME];
	int visible;				/* Show/hide flag */
	Sint16 xinc, yinc;			/* Rendering direction */
	Uint8 alpha;				/* Transparency value */
} AG_MapLayer;

enum ag_map_camera_alignment {
	AG_MAP_UPPER_LEFT,
	AG_MAP_MIDDLE_LEFT,
	AG_MAP_LOWER_LEFT,
	AG_MAP_UPPER_RIGHT,
	AG_MAP_MIDDLE_RIGHT,
	AG_MAP_LOWER_RIGHT,
	AG_MAP_CENTER,
	AG_MAP_LOWER_CENTER,
	AG_MAP_UPPER_CENTER
};

typedef struct ag_map_camera {
	char name[AG_MAP_MAXCAMERANAME];
	int flags;
	int x, y;				/* Position of camera */
	enum ag_map_camera_alignment alignment;	/* View alignment */
	u_int zoom;				/* Zoom (%) */
	int tilesz;				/* Tile size */
	int pixsz;				/* Scaled pixel size */
} AG_MapCamera;

typedef struct ag_mapmod {
	enum {
		AG_MAPMOD_NODECHG,
		AG_MAPMOD_LAYERADD,
		AG_MAPMOD_LAYERDEL
	} type;
	union {
		struct {
			AG_Node node;
			int x, y;
		} nodechg;
		struct {
			int nlayer;
		} layermod;
	} data;
#define mm_nodechg data.nodechg
#define mm_layeradd data.layermod
#define mm_layerdel data.layermod
} AG_MapMod;

typedef struct ag_mapmodblk {
	AG_MapMod *mods;
	u_int nmods;
	u_char cancel;
} AG_MapModBlk;

typedef struct ag_map {
	AG_Space space;

	pthread_mutex_t lock;
	u_int flags;
#define AG_MAP_SAVE_CAM0POS	0x01	/* Save the camera 0 position */
#define AG_MAP_SAVE_CAM0ZOOM	0x02	/* Save the camera 0 zoom factor */
#define AG_MAP_SAVED_FLAGS	(AG_MAP_SAVE_CAM0POS|AG_MAP_SAVE_CAM0ZOOM)

	u_int mapw, maph;		/* Map geometry */
	int cur_layer;			/* Layer being edited */
	struct {
		int x, y;		/* Origin coordinates */
		int layer;		/* Default sprite layer */
	} origin;
	AG_Node **map;			/* Arrays of nodes */
	int redraw;			/* Redraw (for tile-based mode) */
	AG_MapLayer *layers;		/* Layer descriptions */
	u_int nlayers;
	AG_MapCamera *cameras;		/* Views */
	u_int ncameras;
	AG_MapModBlk *blks;		/* Saved modifications */
	u_int nblks;
	u_int curblk;
	u_int nmods;
} AG_Map;

__BEGIN_DECLS
AG_Map	*AG_MapNew(void *, const char *);
void	 AG_MapInit(void *, const char *);
void	 AG_MapReinit(void *);
int	 map_load(void *, AG_Netbuf *);
int	 map_save(void *, AG_Netbuf *);
void	 map_destroy(void *);
void	*map_edit(void *);

int	 AG_MapAllocNodes(AG_Map *, u_int, u_int);
void	 AG_MapFreeNodes(AG_Map *);
int	 AG_MapResize(AG_Map *, u_int, u_int);
void	 AG_MapSetZoom(AG_Map *, int, u_int);
int	 AG_MapPushLayer(AG_Map *, const char *);
void	 AG_MapPopLayer(AG_Map *);
void	 AG_MapInitLayer(AG_MapLayer *, const char *);
void	 AG_MapInitCamera(AG_MapCamera *, const char *);
void	 AG_MapInitModBlks(AG_Map *);
int	 AG_MapAddCamera(AG_Map *, const char *);

void	 AG_MapmodBegin(AG_Map *);
void	 AG_MapmodEnd(AG_Map *);
void	 AG_MapmodCancel(AG_Map *);
void	 AG_MapmodNodeChg(AG_Map *, int, int);
void	 AG_MapmodLayerAdd(AG_Map *, int);
void	 map_undo(AG_Map *);
void	 map_redo(AG_Map *);

void		 AG_NitemInit(AG_Nitem *, enum ag_nitem_type);
__inline__ void	 AG_NitemSetCenter(AG_Nitem *, int, int);
__inline__ void	 AG_NitemSetMotion(AG_Nitem *, int, int);
__inline__ void	 AG_NitemSetFriction(AG_Nitem *, int);
__inline__ void	 AG_NitemSetLayer(AG_Nitem *, int);

void	 	 AG_NitemDestroy(AG_Map *, AG_Nitem *);
int		 AG_NitemLoad(AG_Map *, AG_Netbuf *, AG_Node *,
			      AG_Nitem **);
void	 	 AG_NitemSave(AG_Map *, AG_Netbuf *, AG_Nitem *);
__inline__ int	 AG_NitemExtent(AG_Map *, AG_Nitem *, SDL_Rect *, int);
__inline__ void	 AG_NitemDraw(AG_Map *, AG_Nitem *, int, int, int);
__inline__ void	 AG_NitemSetSprite(AG_Nitem *, AG_Map *, void *, Uint32);
__inline__ void	 AG_NitemSetAnim(AG_Nitem *, AG_Map *, void *, Uint32);
AG_Nitem	*AG_NitemLocate(AG_Map *, int, int, int);
void		 AG_NitemAttrColor(u_int, int, Uint8 *);

__inline__ void	 AG_NodeInit(AG_Node *);
int		 AG_NodeLoad(AG_Map *, AG_Netbuf *, AG_Node *);
void		 AG_NodeSave(AG_Map *, AG_Netbuf *, AG_Node *);
void		 AG_NodeDestroy(AG_Map *, AG_Node *);
void		 AG_NodeRemoveAll(AG_Map *, AG_Node *, int);

__inline__ void	 AG_NodeCopy(AG_Map *, AG_Node *, int, AG_Map *, AG_Node *,
		             int);
void		 AG_NodeMoveItem(AG_Map *, AG_Node *, AG_Nitem *, AG_Map *,
		                 AG_Node *, int);
AG_Nitem	*AG_NodeCopyItem(const AG_Nitem *, AG_Map *, AG_Node *, int);
void		 AG_NodeMoveItemUp(AG_Node *, AG_Nitem *);
void		 AG_NodeMoveItemDown(AG_Node *, AG_Nitem *);
void		 AG_NodeMoveItemToTail(AG_Node *, AG_Nitem *);
void		 AG_NodeMoveItemToHead(AG_Node *, AG_Nitem *);
void		 AG_NodeDelItem(AG_Map *, AG_Node *, AG_Nitem *);
void		 AG_NodeSwapLayers(AG_Map *, AG_Node *, int, int);

AG_Nitem	*AG_NodeAddSprite(AG_Map *, AG_Node *, void *, Uint32);
AG_Nitem	*AG_NodeAddAnim(AG_Map *, AG_Node *, void *, Uint32);
AG_Nitem	*AG_NodeAddWarpPoint(AG_Map *, AG_Node *, const char *, int,
		                     int, Uint8);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_H_ */
