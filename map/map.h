/*	Public domain	*/

#ifndef _AGAR_MAP_MAP_H_
#define _AGAR_MAP_MAP_H_

#include <agar/map/rg_tileset.h>

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

#include <agar/map/begin.h>

/* Type of item contained in a MAP_Node stack. */
enum map_item_type {
	MAP_ITEM_TILE,		/* Reference to a tile */
	MAP_ITEM_WARP		/* Reference to another location */
};

/*
 * TODO: ag_objectify map items. make it possible to create
 * custom classes of items on the fly.
 */
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
	Uint8 _pad1[6];
	void *_Nullable p;		/* User pointer (non-persistent) */
	struct {
		Sint16 xcenter, ycenter;	/* Centering offsets */
		Sint16 xmotion, ymotion;	/* Motion offsets */
		Sint16 xorigin, yorigin;	/* Origin point */
		AG_Rect rs;			/* Source rectangle */
	} r_gfx;
	Uint32 _pad2;
	union {
		struct {
			RG_Tileset *_Nonnull obj;	/* Tileset object */
			Uint id;			/* Tile ID */
			Uint32 _pad;
		} tile;
		struct {
			char *_Nullable map;		/* Target map name */
			int x, y;			/* At coordinates */
			Uint dir;			/* Towards direction */
			Uint32 _pad;
		} warp;
	} nref;
	RG_TransformChain transforms;		/* Graphical transformations */
	MAP_NodeMaskQ masks;			/* Collision detection masks */
	AG_TAILQ_ENTRY(map_item) nrefs;		/* Node's reference stack */
#define r_tile		nref.tile
#define r_warp		nref.warp
} MAP_Item;

AG_TAILQ_HEAD(map_itemq, map_item);

typedef struct map_node {
	struct map_itemq nrefs;		/* Items on this node */
} MAP_Node;

typedef struct map_layer {
	char name[MAP_LAYER_NAME_MAX];
	int visible;				/* Show/hide flag */
	Sint16 xinc, yinc;			/* Rendering direction */
	Uint alpha;				/* 8-bit transparency value */
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

enum map_mod_type {
	AG_MAPMOD_NODECHG,
	AG_MAPMOD_LAYERADD,
	AG_MAPMOD_LAYERDEL
};

typedef struct map_mod {
	enum map_mod_type type;
	Uint32 _pad;
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
	MAP_Mod *_Nonnull mods;
	Uint             nMods;
	int               cancel;
} MAP_ModBlk;

struct map_actor;

typedef struct map {
	struct ag_object obj;		/* AG_Object -> MAP */
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
	int redraw;				/* Redraw (for tile-based mode) */
	MAP_Node *_Nullable *_Nonnull map;	/* Arrays of nodes */
	MAP_Layer *_Nonnull layers;		/* List of layers */
	Uint               nLayers;
	Uint                nCameras;
	MAP_Camera *_Nonnull cameras;		/* List of cameras */
	MAP_ModBlk *_Nonnull blks;		/* Saved modifications */
	Uint                nBlks;
	Uint              curBlk;
	Uint                nMods;	
	Uint32 _pad;
	AG_TAILQ_HEAD_(map_actor) actors;	/* Active objects */
} MAP;

#define MAPMAP(obj)        ((MAP *)(obj))
#define MAPCMAP(obj)       ((const MAP *)(obj))
#define MAP_SELF()          MAPMAP( AG_OBJECT(0,"MAP:*") )
#define MAP_PTR(n)          MAPMAP( AG_OBJECT((n),"MAP:*") )
#define MAP_NAMED(n)        MAPMAP( AG_OBJECT_NAMED((n),"MAP:*") )
#define MAP_CONST_SELF()   MAPCMAP( AG_CONST_OBJECT(0,"MAP:*") )
#define MAP_CONST_PTR(n)   MAPCMAP( AG_CONST_OBJECT((n),"MAP:*") )
#define MAP_CONST_NAMED(n) MAPCMAP( AG_CONST_OBJECT_NAMED((n),"MAP:*") )

__BEGIN_DECLS
extern AG_ObjectClass mapClass;

void MAP_InitSubsystem(void);
void MAP_DestroySubsystem(void);

MAP *_Nonnull MAP_New(void *_Nullable, const char *_Nonnull);

int  MAP_AllocNodes(MAP *_Nonnull, Uint,Uint);
void MAP_FreeNodes(MAP *_Nonnull);
int  MAP_Resize(MAP *_Nonnull, Uint,Uint);
void MAP_SetZoom(MAP *_Nonnull, int, Uint);
int  MAP_PushLayer(MAP *_Nonnull, const char *_Nonnull);
void MAP_PopLayer(MAP *_Nonnull);
void MAP_InitLayer(MAP_Layer *_Nonnull, const char *_Nonnull);
void MAP_InitCamera(MAP_Camera *_Nonnull, const char *_Nonnull);
void MAP_InitModBlks(MAP *_Nonnull);
int  MAP_AddCamera(MAP *_Nonnull, const char *_Nonnull);

void MAP_ModBegin(MAP *_Nonnull);
void MAP_ModEnd(MAP *_Nonnull);
void MAP_ModCancel(MAP *_Nonnull);
void MAP_ModNodeChg(MAP *_Nonnull, int,int);
void MAP_ModLayerAdd(MAP *_Nonnull, int);
void MAP_Undo(MAP *_Nonnull);
void MAP_Redo(MAP *_Nonnull);

void MAP_ItemInit(MAP_Item *_Nonnull, enum map_item_type);
void MAP_ItemSetCenter(MAP_Item *_Nonnull, int,int);
void MAP_ItemSetMotion(MAP_Item *_Nonnull, int,int);
void MAP_ItemSetFriction(MAP_Item *_Nonnull, int);
void MAP_ItemSetLayer(MAP_Item *_Nonnull, int);

void MAP_ItemDestroy(MAP *_Nonnull, MAP_Item *_Nonnull);
int  MAP_ItemLoad(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull,
                  MAP_Item *_Nonnull *_Nonnull);
void MAP_ItemSave(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Item *_Nonnull);
int  MAP_ItemExtent(MAP *_Nonnull, MAP_Item *_Nonnull, AG_Rect *_Nonnull, int);
void MAP_ItemDraw(MAP *_Nonnull, MAP_Item *_Nonnull, int,int, int);

void MAP_PageIn(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);
void MAP_PageOut(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);

void MAP_ItemSetTile(MAP_Item *_Nonnull, MAP *_Nonnull,
                     RG_Tileset *_Nonnull, Uint);
void MAP_ItemAttrColor(Uint, int, AG_Color *_Nonnull);

MAP_Item *_Nullable MAP_ItemLocate(MAP *_Nonnull, int,int, int);

void MAP_NodeInit(MAP_Node *_Nonnull);
int  MAP_NodeLoad(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeSave(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeDestroy(MAP *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeRemoveAll(MAP *_Nonnull, MAP_Node *_Nonnull, int);

void MAP_NodeCopy(MAP *_Nonnull, MAP_Node *_Nonnull, int,
                  MAP *_Nonnull, MAP_Node *_Nonnull, int);
void MAP_NodeMoveItem(MAP *_Nonnull, MAP_Node *_Nonnull, MAP_Item *_Nonnull,
                      MAP *_Nonnull, MAP_Node *_Nonnull, int);

MAP_Item *_Nonnull MAP_NodeCopyItem(const MAP_Item *_Nonnull, MAP *_Nonnull,
                                    MAP_Node *_Nonnull, int);

void MAP_NodeMoveItemUp(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemDown(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToTail(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToHead(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeDelItem(MAP *_Nonnull, MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeSwapLayers(MAP *_Nonnull, MAP_Node *_Nonnull, int,int);

MAP_Item *_Nonnull MAP_NodeAddTile(MAP *_Nonnull, MAP_Node *_Nonnull,
                                   RG_Tileset *_Nonnull, Uint32);
MAP_Item *_Nonnull MAP_NodeAddWarpPoint(MAP *_Nonnull, MAP_Node *_Nonnull,
                                        const char *_Nonnull, int,int, Uint);

void MAP_AttachActor(MAP *_Nonnull, struct map_actor *_Nonnull);
void MAP_DetachActor(MAP *_Nonnull, struct map_actor *_Nonnull);
__END_DECLS

#include <agar/map/close.h>

#include <agar/map/actor.h>
#include <agar/map/tool.h>
#include <agar/map/mapview.h>
#include <agar/map/tools.h>
#include <agar/map/mapedit.h>
#include <agar/map/icons.h>
#include <agar/map/map_math.h>

#endif /* _AGAR_MAP_MAP_H_ */
