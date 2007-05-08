/*	$Csoft: map.h,v 1.20 2005/09/20 13:46:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAP_H_
#define _AGAR_MAP_H_

#define MAPTILESZ 16		/* Default tile size in pixels */

#define MAP_TILESZ_MAX		16384
#define MAP_WIDTH_MAX		32767
#define MAP_HEIGHT_MAX		32767
#define MAP_LAYERS_MAX		256
#define MAP_CAMERAS_MAX		256
#define MAP_LAYER_NAME_MAX	128
#define MAP_CAMERA_NAME_MAX	128
#define MAP_NODE_ITEMS_MAX	32767
#define MAP_ITEM_MAXMASKS	16384

#include <agar/map/nodemask.h>

#include <agar/rg/tileset.h>

#include "begin_code.h"

enum map_item_type {
	MAP_ITEM_TILE,		/* Reference to a tile */
	MAP_ITEM_ANIM,		/* Reference to an animation */
	MAP_ITEM_WARP		/* Reference to another location */
};

typedef struct map_item {
	enum map_item_type type;	/* Type of element */
	
	Uint flags;
#define MAP_ITEM_BLOCK		0x001	/* Tile block */
#define MAP_ITEM_CLIMBABLE	0x002	/* Surface is climbable */
#define MAP_ITEM_SLIPPERY	0x004	/* Surface is slippery */
#define MAP_ITEM_JUMPABLE	0x008	/* Element is jumpable */
#define MAP_ITEM_NOSAVE		0x100	/* Non persistent */
#define MAP_ITEM_MOUSEOVER	0x200	/* Mouse overlap (for editor) */
#define MAP_ITEM_SELECTED	0x400	/* Selection (for editor) */

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
			RG_Tileset *obj;	/* Tileset object */
			Uint32 id;		/* Tile ID */
		} tile;
		struct {
			RG_Tileset *obj;	/* Tileset object */
			Uint32 id;		/* Animation ID */
			Uint *curframe;		/* Current frame number */
		} anim;
		struct {
			char *map;		/* Map identifier */
			int x, y;		/* Origin override */
			Uint8 dir;		/* Default direction */
		} warp;
	} nref;
	RG_TransformChain transforms;		/* Graphical transformations */
	MAP_NodeMaskQ masks;			/* Collision detection masks */
	TAILQ_ENTRY(map_item) nrefs;		/* Node's reference stack */
#define r_tile		nref.tile
#define r_anim		nref.anim
#define r_warp		nref.warp
} MAP_Item;

TAILQ_HEAD(map_itemq, map_item);

typedef struct map_node {
	struct map_itemq nrefs;		/* Items on this node */
} MAP_Node;

typedef struct map_layer {
	char name[MAP_LAYER_NAME_MAX];
	int visible;				/* Show/hide flag */
	Sint16 xinc, yinc;			/* Rendering direction */
	Uint8 alpha;				/* Transparency value */
} MAP_Layer;

enum map_camera_alignment {
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

typedef struct map_camera {
	char name[MAP_CAMERA_NAME_MAX];
	int flags;
	int x, y;				/* Position of camera */
	enum map_camera_alignment alignment;	/* View alignment */
	Uint zoom;				/* Zoom (%) */
	int tilesz;				/* Tile size */
	int pixsz;				/* Scaled pixel size */
} MAP_Camera;

typedef struct map_mod {
	enum {
		AG_MAPMOD_NODECHG,
		AG_MAPMOD_LAYERADD,
		AG_MAPMOD_LAYERDEL
	} type;
	union {
		struct {
			MAP_Node node;
			int x, y;
		} nodechg;
		struct {
			int nlayer;
		} layermod;
	} data;
#define mm_nodechg data.nodechg
#define mm_layeradd data.layermod
#define mm_layerdel data.layermod
} MAP_Mod;

typedef struct map_mod_blk {
	MAP_Mod *mods;
	Uint nmods;
	Uchar cancel;
} MAP_ModBlk;

struct map_actor;

typedef struct map {
	struct ag_object obj;

	AG_Mutex lock;
	Uint flags;
#define AG_MAP_SAVE_CAM0POS	0x01	/* Save the camera 0 position */
#define AG_MAP_SAVE_CAM0ZOOM	0x02	/* Save the camera 0 zoom factor */
#define AG_MAP_SAVED_FLAGS	(AG_MAP_SAVE_CAM0POS|AG_MAP_SAVE_CAM0ZOOM)

	Uint mapw, maph;		/* Map geometry */
	int cur_layer;			/* Layer being edited */
	struct {
		int x, y;		/* Origin coordinates */
		int layer;		/* Default tile layer */
	} origin;
	MAP_Node **map;			/* Arrays of nodes */
	int redraw;			/* Redraw (for tile-based mode) */
	MAP_Layer *layers;		/* Layer descriptions */
	Uint nlayers;
	MAP_Camera *cameras;		/* Views */
	Uint ncameras;
	MAP_ModBlk *blks;		/* Saved modifications */
	Uint nblks;
	Uint curblk;
	Uint nmods;
	TAILQ_HEAD(, map_actor) actors;	/* Active objects */
} MAP;

__BEGIN_DECLS
void	 MAP_InitSubsystem(void);

MAP	*MAP_New(void *, const char *);
void	 MAP_Init(void *, const char *);
void	 MAP_Reinit(void *);
int	 MAP_Load(void *, AG_Netbuf *);
int	 MAP_Save(void *, AG_Netbuf *);
void	 MAP_Destroy(void *);
void	*MAP_Edit(void *);

int	 MAP_AllocNodes(MAP *, Uint, Uint);
void	 MAP_FreeNodes(MAP *);
int	 MAP_Resize(MAP *, Uint, Uint);
void	 MAP_SetZoom(MAP *, int, Uint);
int	 MAP_PushLayer(MAP *, const char *);
void	 MAP_PopLayer(MAP *);
void	 MAP_InitLayer(MAP_Layer *, const char *);
void	 MAP_InitCamera(MAP_Camera *, const char *);
void	 MAP_InitModBlks(MAP *);
int	 MAP_AddCamera(MAP *, const char *);

void	 MAP_ModBegin(MAP *);
void	 MAP_ModEnd(MAP *);
void	 MAP_ModCancel(MAP *);
void	 MAP_ModNodeChg(MAP *, int, int);
void	 MAP_ModLayerAdd(MAP *, int);
void	 MAP_Undo(MAP *);
void	 MAP_Redo(MAP *);

void		 MAP_ItemInit(MAP_Item *, enum map_item_type);
__inline__ void	 MAP_ItemSetCenter(MAP_Item *, int, int);
__inline__ void	 MAP_ItemSetMotion(MAP_Item *, int, int);
__inline__ void	 MAP_ItemSetFriction(MAP_Item *, int);
__inline__ void	 MAP_ItemSetLayer(MAP_Item *, int);

void	 	 MAP_ItemDestroy(MAP *, MAP_Item *);
int		 MAP_ItemLoad(MAP *, AG_Netbuf *, MAP_Node *,
			      MAP_Item **);
void	 	 MAP_ItemSave(MAP *, AG_Netbuf *, MAP_Item *);
__inline__ int	 MAP_ItemExtent(MAP *, MAP_Item *, SDL_Rect *, int);
__inline__ void	 MAP_ItemDraw(MAP *, MAP_Item *, int, int, int);
__inline__ void	 MAP_ItemSetTile(MAP_Item *, MAP *, RG_Tileset *, Uint32);
__inline__ void	 MAP_ItemSetAnim(MAP_Item *, MAP *, RG_Tileset *, Uint32);
MAP_Item	*MAP_ItemLocate(MAP *, int, int, int);
void		 MAP_ItemAttrColor(Uint, int, Uint8 *);

__inline__ void	 MAP_NodeInit(MAP_Node *);
int		 MAP_NodeLoad(MAP *, AG_Netbuf *, MAP_Node *);
void		 MAP_NodeSave(MAP *, AG_Netbuf *, MAP_Node *);
void		 MAP_NodeDestroy(MAP *, MAP_Node *);
void		 MAP_NodeRemoveAll(MAP *, MAP_Node *, int);

__inline__ void	 MAP_NodeCopy(MAP *, MAP_Node *, int, MAP *, MAP_Node *,
		             int);
void		 MAP_NodeMoveItem(MAP *, MAP_Node *, MAP_Item *, MAP *,
		                 MAP_Node *, int);
MAP_Item	*MAP_NodeCopyItem(const MAP_Item *, MAP *, MAP_Node *, int);
void		 MAP_NodeMoveItemUp(MAP_Node *, MAP_Item *);
void		 MAP_NodeMoveItemDown(MAP_Node *, MAP_Item *);
void		 MAP_NodeMoveItemToTail(MAP_Node *, MAP_Item *);
void		 MAP_NodeMoveItemToHead(MAP_Node *, MAP_Item *);
void		 MAP_NodeDelItem(MAP *, MAP_Node *, MAP_Item *);
void		 MAP_NodeSwapLayers(MAP *, MAP_Node *, int, int);

MAP_Item	*MAP_NodeAddTile(MAP *, MAP_Node *, RG_Tileset *, Uint32);
MAP_Item	*MAP_NodeAddAnim(MAP *, MAP_Node *, RG_Tileset *, Uint32);
MAP_Item	*MAP_NodeAddWarpPoint(MAP *, MAP_Node *, const char *, int,
		                     int, Uint8);

void	 	 MAP_AttachActor(MAP *, struct map_actor *);
void	 	 MAP_DetachActor(MAP *, struct map_actor *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAP_H_ */
