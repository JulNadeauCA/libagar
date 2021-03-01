/*	Public domain	*/

#ifndef _AGAR_MAP_MAP_H_
#define _AGAR_MAP_MAP_H_

#include <agar/map/rg_tileset.h>

#define MAPTILESZ 64		/* Default tile size in pixels */

#define MAP_TILESZ_MAX		16384
#define MAP_WIDTH_MAX		32767
#define MAP_HEIGHT_MAX		32767
#define MAP_LAYERS_MAX		256
#define MAP_CAMERAS_MAX		256
#define MAP_LAYER_NAME_MAX	128
#define MAP_CAMERA_NAME_MAX	128
#define MAP_NODE_ITEMS_MAX	32767

#include <agar/map/begin.h>

struct map;
struct map_node;
struct map_view;
struct map_item;

/* Map item type */
enum map_item_type {
	MAP_ITEM_TILE        = 0,	/* RG_Tile(3) element */
	MAP_ITEM_IMG         = 1,	/* Image file fragment */
	MAP_ITEM_LINK        = 2,	/* Link to a map location */
	MAP_ITEM_PRIVATE     = 3,	/* Start of private range */
/*	MAP_ITEM_MY_TYPE1    = 4, */
/*	MAP_ITEM_MY_TYPE2    = 5, */
	MAP_ITEM_LAST        = 20
};

/* Map item class description */
typedef struct map_item_class {
	const char *name;                    /* Display name */
	const char *url;                     /* Project URL */
	const char *descr;                   /* Display description */
	enum map_item_type type;
	Uint flags;
	AG_Size size;                        /* Instance structure size (B) */

	/* Initialization and finalization */
	void  (*_Nullable init)(void *_Nonnull);
	void  (*_Nullable destroy)(void *_Nonnull);

	/* Duplicate item onto a target node */
	void *_Nonnull (*_Nonnull  duplicate)(struct map *_Nonnull,
	                                      struct map_node *_Nonnull,
	                                      const void *_Nonnull);

	/* Serialization */
	int   (*_Nullable load)(struct map *_Nonnull, void *_Nonnull,
	                        AG_DataSource *_Nonnull);
	void  (*_Nullable save)(struct map *_Nonnull, void *_Nonnull,
	                        AG_DataSource *_Nonnull);

	/* Rendering routine (rendering context) */
	void  (*_Nullable draw)(struct map_view *_Nonnull,
	                        struct map_item *_Nonnull,
	                        int,int, int);
	
	/* Oriented bounding box (relative to camera) */
	int   (*_Nullable extent)(struct map *_Nonnull, void *_Nonnull,
	                          AG_Rect *_Nonnull, int);

	/* Pixel collision test (relative to camera) */
	int   (*_Nonnull collide)(void *_Nonnull, int,int);

	/* Edition */
	void *_Nullable (*_Nullable edit)(void *_Nonnull,
	                                  struct map_view *_Nonnull);
} MAP_ItemClass;

/* Map item (node element) */
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

	Uint layer;			/* Associated layer */
	Uint32 _pad1;
	void *_Nullable p;		/* User pointer (non-persistent) */
	RG_TransformChain transforms;	/* Graphical transformations */
	AG_TAILQ_ENTRY(map_item) items;	/* Node's reference stack */
} MAP_Item;

AG_TAILQ_HEAD(map_itemq, map_item);

typedef struct map_node {
	struct map_itemq items;			/* Items on this node */
} MAP_Node;

typedef struct map_layer {
	char name[MAP_LAYER_NAME_MAX];
	int visible;				/* Show/hide flag */
	int xinc, yinc;				/* Rendering direction */
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
	char name[MAP_CAMERA_NAME_MAX];		/* Name identifier */
	Uint flags;
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
	struct ag_object obj;			/* AG_Object -> MAP */

	Uint flags;
#define AG_MAP_SAVE_CAM0POS	0x01		/* Save cam0 position */
#define AG_MAP_SAVE_CAM0ZOOM	0x02		/* Save cam0 zoom factor */
#define AG_MAP_SAVED_FLAGS	(AG_MAP_SAVE_CAM0POS | AG_MAP_SAVE_CAM0ZOOM)

	Uint w, h;				/* Map size (in nodes) */
	int layerCur;				/* Layer being edited */
	int xOrigin, yOrigin;			/* Origin node */
	int layerOrigin;			/* Origin node layer# */
	Uint32 _pad1;
	MAP_Node *_Nullable *_Nonnull map;	/* Arrays of nodes */
	MAP_Layer *_Nonnull layers;		/* Layer information */
	Uint               nLayers;		/* Layer count */
	Uint                nCameras;		/* Camera count */
	MAP_Camera *_Nonnull cameras;		/* Cameras (viewports) */
	MAP_ModBlk *_Nonnull blks;		/* Undo levels */
	Uint                nBlks;		/* Undo level count */
	Uint              curBlk;		/* Current undo level index */      
	Uint                nMods;		/* Modifications in this block */
	Uint32 _pad2;
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

#define MAPITEM(mi)       ((MAP_Item *)(mi))

__BEGIN_DECLS
extern AG_ObjectClass mapClass;
extern MAP_ItemClass *mapItemClasses[MAP_ITEM_LAST];

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

void MAP_ItemInit(void *_Nonnull, enum map_item_type);

void MAP_ItemDestroy(MAP *_Nonnull, MAP_Item *_Nonnull);
int  MAP_ItemLoad(MAP *_Nonnull, MAP_Node *_Nonnull, AG_DataSource *_Nonnull);
void MAP_ItemSave(MAP *_Nonnull, MAP_Item *_Nonnull, AG_DataSource *_Nonnull);
int  MAP_ItemExtent(MAP *_Nonnull, const MAP_Item *_Nonnull, AG_Rect *_Nonnull, int);
void MAP_ItemDraw(MAP *_Nonnull, MAP_Item *_Nonnull, int,int, int);

void MAP_PageIn(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);
void MAP_PageOut(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);

void MAP_ItemAttrColor(Uint, int, AG_Color *_Nonnull);

MAP_Item *_Nullable MAP_ItemLocate(MAP *_Nonnull, int,int, int);

void MAP_NodeInit(MAP_Node *_Nonnull);
int  MAP_NodeLoad(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeSave(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeDestroy(MAP *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeRemoveAll(MAP *_Nonnull, MAP_Node *_Nonnull, int);

void MAP_NodeCopy(MAP *_Nonnull, MAP_Node *_Nonnull, int,
                  MAP *_Nonnull, MAP_Node *_Nonnull, int);

MAP_Item *_Nonnull MAP_NodeDuplicate(const MAP_Item *_Nonnull, MAP *_Nonnull,
                                     MAP_Node *_Nonnull, int);

void MAP_NodeMoveItemUp(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemDown(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToTail(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToHead(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeDelItem(MAP *_Nonnull, MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeSwapLayers(MAP *_Nonnull, MAP_Node *_Nonnull, int,int);

void MAP_AttachActor(MAP *_Nonnull, struct map_actor *_Nonnull);
void MAP_DetachActor(MAP *_Nonnull, struct map_actor *_Nonnull);
__END_DECLS

#include <agar/map/map_tile.h>
#include <agar/map/map_img.h>
#include <agar/map/map_link.h>

#include <agar/map/close.h>

#include <agar/map/actor.h>
#include <agar/map/tool.h>
#include <agar/map/mapview.h>
#include <agar/map/tools.h>
#include <agar/map/mapedit.h>
#include <agar/map/icons.h>
#include <agar/map/map_math.h>

#endif /* _AGAR_MAP_MAP_H_ */
