/*	Public domain	*/

#ifndef _AGAR_MAP_MAP_H_
#define _AGAR_MAP_MAP_H_

#include <agar/map/rg_tileset.h>

#ifndef MAP_TILESZ_DEF
#define MAP_TILESZ_DEF      32		/* Default tile size in pixels */
#endif
#ifndef MAP_TILESZ_MAX
#define MAP_TILESZ_MAX      16384
#endif
#ifndef MAP_WIDTH_MAX
#define MAP_WIDTH_MAX       32767
#endif
#ifndef MAP_HEIGHT_MAX
#define MAP_HEIGHT_MAX      32767
#endif
#define MAP_LAYERS_MAX      256
#define MAP_CAMERAS_MAX     256
#define MAP_LAYER_NAME_MAX  128
#define MAP_CAMERA_NAME_MAX 128
#define MAP_NODE_ITEMS_MAX  32767
#define MAP_OBJECT_ID_MAX   0x7fffff

#include <agar/map/begin.h>

struct map;
struct map_node;
struct map_view;
struct map_item;
struct map_object;
struct map_location;

/* Static map item type */
enum map_item_type {
	MAP_ITEM_TILE    = 0,	/* RG_Tile(3) element */
	MAP_ITEM_IMG     = 1,	/* Image file fragment */
	MAP_ITEM_LINK    = 2,	/* Link to a map location */
	MAP_ITEM_PRIVATE = 3,	/* User-defined range */
	MAP_ITEM_LAST    = 4
};

/* Static map item class description */
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

/*
 * Static map item. Built-in item types include:
 * 
 *	- Structured Tile Reference with Transforms (TILE)
 *	- Image File Reference with Transforms (IMG)
 *	- Map Location Reference (LINK)
 *
 * A MAP_Item may only occupy a single node.
 */
typedef struct map_item {
	enum map_item_type type;	/* Type of element */

	Uint flags;
#define MAP_ITEM_BLOCK		0x0001	/* Tile block */
#define MAP_ITEM_CLIMBABLE	0x0002	/* Surface is climbable */
#define MAP_ITEM_SLIPPERY	0x0004	/* Surface is slippery */
#define MAP_ITEM_JUMPABLE	0x0008	/* Element is jumpable */
#define MAP_ITEM_MOUSEOVER	0x0200	/* Mouse overlap (for editor) */
#define MAP_ITEM_SELECTED	0x0400	/* Selection (for editor) */
#define MAP_ITEM_VALID		0x8000	/* Validity flag */

	Uint layer;			/* Layer tag */
	float z;			/* Vertical coordinate */
	float h;			/* Height of item */
	Uint32 _pad;
	void *_Nullable p;		/* User pointer (non-persistent) */
	RG_TransformChain transforms;	/* Graphical transformations */
	AG_TAILQ_ENTRY(map_item) items;	/* Node's reference stack */
} MAP_Item;

AG_TAILQ_HEAD(map_itemq, map_item);

typedef struct map_node {
	Uint flags;
#define MAP_NODE_SELECTED 0x0002       /* Selected in Editor */
#define MAP_NODE_VALID    0x4000       /* Validity flag */

	Uint                          nLocs;  /* Object location count */
	struct map_location *_Nullable locs;  /* Object locations */
	struct map_itemq               items; /* Static items */
} MAP_Node;

typedef struct map_layer {
	char name[MAP_LAYER_NAME_MAX];
	int visible;				/* Show/hide flag */
	int xinc, yinc;				/* Rendering direction */
	Uint alpha;				/* 8-bit transparency value */
} MAP_Layer;

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

typedef struct map_camera {
	char name[MAP_CAMERA_NAME_MAX];		/* Name identifier */
	Uint flags;
	int x, y;				/* Position of camera */
	enum map_camera_alignment alignment;	/* View alignment */
	Uint zoom;				/* Zoom (%) */
	int tilesz;				/* Tile size */
	float pixsz;				/* Scaled pixel size */
} MAP_Camera;

enum map_change_type {
	MAP_CHANGE_NODECHG,
	MAP_CHANGE_LAYERADD,
	MAP_CHANGE_LAYERDEL,
	MAP_CHANGE_LAST
};

typedef struct map_change {
	enum map_change_type type;
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
} MAP_Change;

typedef struct map_revision {
	MAP_Change *_Nonnull changes;           /* Changes array */
	Uint                nChanges;
	int cancel;                             /* Revert flag */
} MAP_Revision;

typedef struct map {
	struct ag_object obj;			/* AG_Object -> MAP */

	Uint flags;
#define MAP_SAVE_CAM0POS  0x01			/* Save cam0 position */
#define MAP_SAVE_CAM0ZOOM 0x02			/* Save cam0 zoom factor */
#define MAP_SAVED_FLAGS	(MAP_SAVE_CAM0POS | MAP_SAVE_CAM0ZOOM)

	Uint w, h;				/* Map size (in nodes) */
	int layerCur;				/* Layer being edited */
	int xOrigin, yOrigin;			/* Origin node */
	int layerOrigin;			/* Origin node layer# */
	Uint nChanges;				/* Changes since last action (for MAP_Tool) */
	MAP_Node *_Nullable *_Nonnull map;	/* Arrays of nodes */
	MAP_Layer *_Nonnull layers;		/* Layer information */
	Uint               nLayers;		/* Layer count */
	Uint                nCameras;		/* Camera count */
	MAP_Camera *_Nonnull cameras;		/* Cameras (viewports) */
	MAP_Revision *_Nonnull undo;		/* History Buffer (Undo stack) */
	MAP_Revision *_Nonnull redo;		/* Redo stack */
	Uint                  nUndo;		/* Undo stack size */
	Uint                  nRedo;		/* Redo stack size */

	/* Map objects */
	Uint                                 nObjs;
	Uint32 _pad;
	struct map_object *_Nonnull *_Nonnull objs;

	AG_Object *_Nullable pLibs;		/* Libraries container object */
	AG_Object *_Nullable pMaps;		/* Maps container object */
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
void MAP_InitHistory(MAP *_Nonnull);
void MAP_ClearHistory(MAP *_Nonnull);
int  MAP_AddCamera(MAP *_Nonnull, const char *_Nonnull);

void MAP_BeginRevision(MAP *_Nonnull);
void MAP_AbortRevision(MAP *_Nonnull);
void MAP_CommitRevision(MAP *_Nonnull);

void MAP_UndoRevision(MAP *_Nonnull, MAP_Revision *_Nonnull,
                      MAP_Revision *_Nonnull, Uint);

void MAP_NodeRevision(MAP *_Nonnull, int,int, MAP_Revision *_Nonnull, Uint);

void MAP_Undo(MAP *_Nonnull);
void MAP_Redo(MAP *_Nonnull);

void MAP_ItemInit(void *_Nonnull, enum map_item_type);

void MAP_ItemDestroy(MAP *_Nonnull, MAP_Item *_Nonnull);
int  MAP_ItemLoad(MAP *_Nonnull, MAP_Node *_Nonnull, AG_DataSource *_Nonnull);
void MAP_ItemSave(MAP *_Nonnull, MAP_Item *_Nonnull, AG_DataSource *_Nonnull);
int  MAP_ItemExtent(MAP *_Nonnull, const MAP_Item *_Nonnull, AG_Rect *_Nonnull, int);

void MAP_PageIn(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);
void MAP_PageOut(MAP *_Nonnull, const char *_Nonnull, void *_Nonnull);

void MAP_ItemAttrColor(Uint, int, AG_Color *_Nonnull);

MAP_Item *_Nullable MAP_ItemLocate(MAP *_Nonnull, int,int, int);

void MAP_NodeInit(MAP_Node *_Nonnull);
int  MAP_NodeLoad(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeSave(MAP *_Nonnull, AG_DataSource *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeDestroy(MAP *_Nonnull, MAP_Node *_Nonnull);
void MAP_NodeClear(MAP *_Nonnull, MAP_Node *_Nonnull, int);
void MAP_NodeCopy(MAP *_Nonnull, MAP_Node *_Nonnull, int, const MAP_Node *, int);

MAP_Item *_Nonnull MAP_DuplicateItem(MAP *_Nonnull, MAP_Node *_Nonnull, int,
                                     const MAP_Item *_Nonnull);

struct map_location *_Nonnull MAP_DuplicateLocation(MAP *, MAP_Node *, int,
                                                    const struct map_location *);

void MAP_NodeMoveItemUp(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemDown(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToTail(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeMoveItemToHead(MAP_Node *_Nonnull, MAP_Item *_Nonnull);
void MAP_NodeDelItem(MAP *_Nonnull, MAP_Node *_Nonnull, MAP_Item *_Nonnull);
int  MAP_NodeDelLocation(MAP *_Nonnull, MAP_Node *_Nonnull, struct map_location *_Nonnull);
int  MAP_NodeDelLocationAtIndex(MAP *_Nonnull, MAP_Node *_Nonnull, int);
void MAP_NodeSwapLayers(MAP *_Nonnull, MAP_Node *_Nonnull, int,int);
__END_DECLS

#include <agar/map/map_object.h>
#include <agar/map/map_tile.h>
#include <agar/map/map_img.h>
#include <agar/map/map_link.h>

#include <agar/map/close.h>

#include <agar/map/tool.h>
#include <agar/map/mapview.h>
#include <agar/map/tools.h>
#include <agar/map/icons.h>
#include <agar/map/map_math.h>

#include <agar/map/nodesel.h>

#endif /* _AGAR_MAP_MAP_H_ */
