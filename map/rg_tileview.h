/*	Public domain	*/

#ifndef _AGAR_RG_TILEVIEW_H_
#define _AGAR_RG_TILEVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/menu.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/box.h>
#include <agar/gui/iconmgr.h>

#include <agar/map/rg_tileset.h>

#include <agar/map/begin.h>

#ifndef RG_TILEVIEW_MIN_W
#define RG_TILEVIEW_MIN_W 32
#endif
#ifndef RG_TILEVIEW_MIN_H
#define RG_TILEVIEW_MIN_H 32
#endif

enum rg_tileview_ctrl_type {
	RG_TILEVIEW_POINT,		/* Point (x,y) */
	RG_TILEVIEW_RECTANGLE,		/* Rectangle (x,y,w,h) */
	RG_TILEVIEW_RDIMENSIONS,	/* Rectangle (w,h) */
	RG_TILEVIEW_CIRCLE,		/* Circle (x,y,r) */
	RG_TILEVIEW_VERTEX,		/* Vg vertex (x,y) */
};

enum tileview_val_type {
	RG_TILEVIEW_INT_VAL,
	RG_TILEVIEW_INT_PTR,
	RG_TILEVIEW_UINT_VAL,
	RG_TILEVIEW_UINT_PTR,
	RG_TILEVIEW_FLOAT_VAL,
	RG_TILEVIEW_FLOAT_PTR,
	RG_TILEVIEW_DOUBLE_VAL,
	RG_TILEVIEW_DOUBLE_PTR,
};

union rg_tileview_val {
	int i;
	Uint ui;
	float f;
	double d;
	void *_Nullable p;
};

typedef struct rg_tileview_handle {
	int x, y;		/* Cached tile coords (set by draw routine) */
	int over;		/* Mouse overlap */
	int enable;		/* Mouse selection */
} RG_TileviewHandle;

typedef struct rg_tileview_ctrl {
	enum rg_tileview_ctrl_type type;

	AG_Color c, cIna, cEna, cOver, cHigh, cLow;
	Uint8    a, aIna, aEna, aOver;

	enum tileview_val_type *_Nonnull valtypes;	/* Entry types */
	union rg_tileview_val  *_Nonnull vals;		/* Values/pointers */
	Uint                             nvals;

	Uint                                nHandles;
	struct rg_tileview_handle *_Nullable handles;	/* User handles */
#if 0
	VG *vg;			/* For RG_TILEVIEW_VERTEX */
	VG_Element *vge;
#endif
	AG_Event *_Nullable motion;
	AG_Event *_Nullable buttonup;
	AG_Event *_Nullable buttondown;

	int xoffs, yoffs;

	AG_TAILQ_ENTRY(rg_tileview_ctrl) ctrls;
} RG_TileviewCtrl;

typedef struct rg_tileview_tool_ops {
	const char *_Nonnull name;	/* Name of tool */
	const char *_Nonnull desc;	/* Tool description */
	AG_Size len;			/* Size of structure */
#if AG_MODEL == AG_LARGE
	Uint64 flags;
#else
	Uint flags;
#endif
	AG_StaticIcon *_Nullable icon;	/* Display icon */
	int cursor;			/* Autoset cursor (or -1) */
	AG_KeySym hotkey;		/* Keyboard shortcut (or NONE) */

	void                (*_Nullable init)(void *_Nonnull);
	void                (*_Nullable destroy)(void *_Nonnull);
	AG_Window *_Nonnull (*_Nullable edit)(void *_Nonnull);
	void                (*_Nullable selected)(void *_Nonnull);
	void                (*_Nullable unselected)(void *_Nonnull);
} RG_TileviewToolOps;

typedef struct rg_tileview_bitmap_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*_Nullable mousebuttondown)(void *_Nonnull, int,int, int);
	void (*_Nullable mousebuttonup)(void *_Nonnull, int,int, int);
	void (*_Nullable mousemotion)(void *_Nonnull, int,int, int,int);
} RG_TileviewBitmapToolOps;

#if 0
typedef struct rg_tileview_sketch_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*_Nullable mousebuttondown)(void *_Nonnull, RG_Sketch *_Nonnull,
	                                  float,float, int);
	void (*_Nullable mousebuttonup)(void *_Nonnull, RG_Sketch *_Nonnull,
	                                float,float, int);
	void (*_Nullable mousemotion)(void *_Nonnull, RG_Sketch *_Nonnull,
	                              float,float, float,float);
	int  (*_Nullable mousewheel)(void *_Nonnull, RG_Sketch *_Nonnull, int);
	void (*_Nullable keydown)(void *_Nonnull, RG_Sketch *_Nonnull, int, int);
	void (*_Nullable keyup)(void *_Nonnull, RG_Sketch *_Nonnull, int, int);
} RG_TileviewSketchToolOps;
#endif

typedef struct rg_tileview_tool {
	const RG_TileviewToolOps *_Nonnull ops;
	struct rg_tileview *_Nonnull tv;
	Uint flags;
#define TILEVIEW_TILE_TOOL	0x01	/* Call in default edition mode */
#define TILEVIEW_FEATURE_TOOL	0x02	/* Call in feature edition mode */
#define TILEVIEW_SKETCH_TOOL	0x04	/* Call in vector edition mode */
#define TILEVIEW_PIXMAP_TOOL	0x08	/* Call in pixmap edition mode */
	Uint32 _pad;
	AG_Window *_Nullable win;	/* Expanded window */
	AG_TAILQ_ENTRY(rg_tileview_tool) tools;
} RG_TileviewTool;

enum rg_tileview_state {
	RG_TILEVIEW_TILE_EDIT,		/* Default edition mode */
	RG_TILEVIEW_FEATURE_EDIT,	/* A feature is being edited */
#if 0
	RG_TILEVIEW_SKETCH_EDIT,	/* A sketch is being edited inline */
#endif
	RG_TILEVIEW_PIXMAP_EDIT,	/* A pixmap is being edited inline */
	RG_TILEVIEW_ATTRIB_EDIT,	/* Node attributes are being edited */
	RG_TILEVIEW_LAYERS_EDIT		/* Node layers are being edited */
};

struct ag_text_cache;

typedef struct rg_tileview {
	AG_Widget wid;
	RG_Tileset *_Nullable ts;	/* Attached tileset */
	RG_Tile    *_Nullable tile;	/* Active tile */
	int zoom;			/* Display zoom (%) */
	int pxsz;			/* Scaled pixel size (pixels) */
	int pxlen;			/* Scaled pixel size (bytes) */
	int xoffs, yoffs;		/* Display offset */
	int xms, yms;			/* Cursor coords in surface (pixels) */
	int xsub, ysub;			/* Cursor subpixel coords (v.pixels) */
	int xorig, yorig;		/* Origin used when moving controls */
	int su;				/* Rendered surface handle (or -1) */
	AG_Surface *_Nullable scaled;	/* Scaled surface (source) */
	int scrolling;
	int flags;
#define RG_TILEVIEW_NO_SCROLLING 0x01	/* Disable right click scrolling */
#define RG_TILEVIEW_HIDE_CONTROLS 0x02	/* Hide the current controls */
#define RG_TILEVIEW_NO_TILING	0x04	/* Don't draw background tiling */
#define RG_TILEVIEW_NO_EXTENT	0x08	/* Hide the extent rectangle */
#define RG_TILEVIEW_NO_GRID	0x10	/* Hide the tile grid */
#define RG_TILEVIEW_SET_ATTRIBS	0x20	/* Setting node attributes */
#define RG_TILEVIEW_READONLY	0x40

	AG_Timer zoomTo;		/* Zoom timeout */
	AG_Timer redrawTo;		/* Auto redraw timeout */

	int edit_attr;			/* Attribute being edited */
	int edit_mode;			/* Element is being edited */
	enum rg_tileview_state state;	/* Current editor mode */
	Uint32 _pad;

	AG_Box       *_Nullable tel_box;	/* Element-specific toolbar container */
	AG_Toolbar   *_Nullable tel_tbar;	/* Element-specific toolbar */
	AG_PopupMenu *_Nullable menu;		/* Popup menu */

	union {
		struct {
			RG_TileElement *_Nonnull tel;	/* Feature element */
			RG_Feature *_Nonnull ft;  
			AG_Window *_Nullable win;	/* Settings */
			AG_PopupMenu *_Nullable menu;	/* Popup menu */
		} feature;
#if 0
		struct {
			RG_TileElement *_Nonnull tel;		/* Sketch element */
			RG_Sketch *_Nonnull sk;
			AG_Window *_Nullable win;		/* Settings */
			RG_TileviewCtrl *_Nullable ctrl;	/* Extent control */
			AG_PopupMenu *_Nullable menu;		/* Popup menu */
		} sketch;
#endif
		struct {
			RG_TileElement *_Nonnull tel;	 /* Pixmap element */
			RG_Pixmap *_Nonnull px;
			AG_Window *_Nullable win;	 /* Settings */
			RG_TileviewCtrl *_Nullable ctrl; /* Extent control */
			enum {
				RG_TVPIXMAP_IDLE,
				RG_TVPIXMAP_FREEHAND,
				RG_TVPIXMAP_ORTHOGONAL,
				RG_TVPIXMAP_VERTICAL,
				RG_TVPIXMAP_HORIZONTAL,
				RG_TVPIXMAP_DIAGONAL
			} state;
			Uint32 _pad;
			AG_PopupMenu *_Nullable menu;	/* Popup menu */
			int xorig, yorig;		/* Ref pt for restrictions */
		} pixmap;
		struct {
			RG_TileviewCtrl *_Nullable geo_ctrl;	/* Geometry control */
			RG_TileviewCtrl *_Nullable orig_ctrl;	/* Origin control */
		} tile;
		struct {
			int nx, ny;			/* Current position */
		} attrs;
	} sargs;
#ifndef _AGAR_RG_PUBLIC_H_
#define tv_feature sargs.feature
#define tv_sketch  sargs.sketch
#define tv_pixmap  sargs.pixmap
#define tv_tile	   sargs.tile
#define tv_attrs   sargs.attrs
#endif
	struct {
		Uint8 r, g, b, a;		/* Current color */
		Uint32 pc;			/* (for binding controls) */
	} c;
	AG_TAILQ_HEAD_(rg_tileview_ctrl) ctrls;	/* Binding controls */
	AG_TAILQ_HEAD_(rg_tileview_tool) tools;	/* Edition tools */
	RG_TileviewTool *_Nullable cur_tool;	/* Active tool */
	struct ag_text_cache *_Nonnull tCache;	/* For "status" line */
} RG_Tileview;

#define RG_TILEVIEW_TOOL(p) ((RG_TileviewTool *)p)
#define RG_TILEVIEW_MAPPED_X(tv, x) ((tv)->xoffs + (x)*(tv)->pxsz)
#define RG_TILEVIEW_MAPPED_Y(tv, y) ((tv)->yoffs + (y)*(tv)->pxsz)
#define RG_TILEVIEW_MAPPED_X_ABS(tv, x) (AGWIDGET(tv)->rView.x1 + (tv)->xoffs + (x)*(tv)->pxsz)
#define RG_TILEVIEW_MAPPED_Y_ABS(tv, y) (AGWIDGET(tv)->rView.y1 + (tv)->yoffs + (y)*(tv)->pxsz)

#define RGTILEVIEW(obj)            ((RG_Tileview *)(obj))
#define RGCTILEVIEW(obj)           ((const RG_Tileview *)(obj))
#define RG_TILEVIEW_SELF()          RGTILEVIEW( AG_OBJECT(0,"AG_Widget:RG_Tileview:*") )
#define RG_TILEVIEW_PTR(n)          RGTILEVIEW( AG_OBJECT((n),"AG_Widget:RG_Tileview:*") )
#define RG_TILEVIEW_NAMED(n)        RGTILEVIEW( AG_OBJECT_NAMED((n),"AG_Widget:RG_Tileview:*") )
#define RG_CONST_TILEVIEW_SELF()   RGCTILEVIEW( AG_CONST_OBJECT(0,"AG_Widget:RG_Tileview:*") )
#define RG_CONST_TILEVIEW_PTR(n)   RGCTILEVIEW( AG_CONST_OBJECT((n),"AG_Widget:RG_Tileview:*") )
#define RG_CONST_TILEVIEW_NAMED(n) RGCTILEVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:RG_Tileview:*") )

__BEGIN_DECLS
extern AG_WidgetClass rgTileviewClass;

RG_Tileview	*_Nonnull RG_TileviewNew(void *_Nullable, RG_Tileset *_Nonnull,
                                         Uint);
RG_TileviewTool *_Nonnull RG_TileviewRegTool(RG_Tileview *_Nonnull,
                                             const void *_Nonnull);

void RG_TileviewSetTile(RG_Tileview *_Nonnull, RG_Tile *_Nonnull);
void RG_TileviewSetZoom(RG_Tileview *_Nonnull, int, int);
void RG_TileviewSetAutoRefresh(RG_Tileview *_Nonnull, int, int);

void RG_TileviewColor3i(RG_Tileview *_Nonnull, Uint8,Uint8,Uint8);
void RG_TileviewColor4i(RG_Tileview *_Nonnull, Uint8,Uint8,Uint8, Uint8);
void RG_TileviewColor(RG_Tileview *_Nonnull, const AG_Color *_Nonnull, Uint8);
void RG_TileviewAlpha(RG_Tileview *_Nonnull, Uint8);

void RG_TileviewPixel(RG_Tileview *_Nonnull, int,int);
void RG_TileviewRect(RG_Tileview *_Nonnull, int,int, int,int);
void RG_TileviewRectOut(RG_Tileview *_Nonnull, int,int, int,int);
void RG_TileviewCircle(RG_Tileview *_Nonnull, int,int, int);
void RG_TileviewHLine(RG_Tileview *_Nonnull, int,int, int);
void RG_TileviewVLine(RG_Tileview *_Nonnull, int, int,int);

RG_TileviewCtrl *_Nonnull RG_TileviewAddCtrl(RG_Tileview *_Nonnull,
                                             enum rg_tileview_ctrl_type,
                                             const char *_Nonnull, ...);

void RG_TileviewDelCtrl(RG_Tileview *_Nonnull, RG_TileviewCtrl *_Nonnull);

void RG_TileviewSelectTool(RG_Tileview *_Nonnull, RG_TileviewTool *_Nonnull);
void RG_TileviewUnselectTool(RG_Tileview *_Nonnull);
void RG_TileviewGenericMenu(RG_Tileview *_Nonnull, AG_MenuItem *_Nonnull);

/* Put (scaled) pixel on the tileview's cache surface. */
static __inline__ void
RG_TileviewPixelCached(RG_Tileview *_Nonnull tv, int x, int y,
    Uint8 r, Uint8 g, Uint8 b)
{
	int sx = x*tv->pxsz;
	int sy = y*tv->pxsz;
	Uint32 pixel;
	Uint8 *dst;

	if (sx < 0 || sy < 0 ||
	    (Uint)sx >= tv->scaled->w || (Uint)sy >= tv->scaled->h) {
		return;
	}
	pixel = AG_MapPixel32_RGB8(&tv->scaled->format, r,g,b);
	if (tv->pxsz == 1) {
		dst = (Uint8 *)tv->scaled->pixels + y*tv->scaled->pitch +
		    x*tv->scaled->format.BytesPerPixel;
		*(Uint32 *)dst = pixel;
	} else {
		int px, py;
		
		dst = (Uint8 *)tv->scaled->pixels +
		    sy*tv->scaled->pitch +
		    sx*tv->scaled->format.BytesPerPixel;
		for (py = 0; py < tv->pxsz; py++) {
			for (px = 0; px < tv->pxsz; px++) {
				*(Uint32 *)dst = pixel;
				dst += tv->scaled->format.BytesPerPixel;
			}
			dst += tv->scaled->pitch - tv->pxlen;
		}
	}
	AG_WidgetUpdateSurface(tv, 0);
}

/*
 * Return values associated with control bindings.
 */
static __inline__ int
RG_TileviewInt(RG_TileviewCtrl *_Nonnull ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_INT_VAL:
		return (ctrl->vals[nval].i);
	case RG_TILEVIEW_INT_PTR:
		return (*(int *)ctrl->vals[nval].p);
	case RG_TILEVIEW_UINT_VAL:
		return ((int)ctrl->vals[nval].ui);
	case RG_TILEVIEW_UINT_PTR:
		return (*(Uint *)ctrl->vals[nval].p);
	default:
		AG_FatalError("cannot convert");
	}
	return (0);
}
static __inline__ float
RG_TileviewFloat(RG_TileviewCtrl *_Nonnull ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		return (ctrl->vals[nval].f);
	case RG_TILEVIEW_FLOAT_PTR:
		return (*(float *)ctrl->vals[nval].p);
	case RG_TILEVIEW_DOUBLE_VAL:
		return ((float)ctrl->vals[nval].d);
	case RG_TILEVIEW_DOUBLE_PTR:
		return ((float)(*(double *)ctrl->vals[nval].p));
	default:
		AG_FatalError("cannot convert");
	}
	return (0.0);
}
static __inline__ double
RG_TileviewDouble(RG_TileviewCtrl *_Nonnull ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		return ((double)ctrl->vals[nval].f);
	case RG_TILEVIEW_FLOAT_PTR:
		return ((double)(*(float *)ctrl->vals[nval].p));
	case RG_TILEVIEW_DOUBLE_VAL:
		return (ctrl->vals[nval].d);
	case RG_TILEVIEW_DOUBLE_PTR:
		return (*(double *)ctrl->vals[nval].p);
	default:
		AG_FatalError("cannot convert");
	}
	return (0.0);
}

/*
 * Set values associated with control bindings.
 */
static __inline__ void
RG_TileviewSetInt(RG_TileviewCtrl *_Nonnull ctrl, int nval, int v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_INT_VAL:
		ctrl->vals[nval].i = v;
		break;
	case RG_TILEVIEW_UINT_VAL:
		ctrl->vals[nval].ui = (Uint)v;
		break;
	case RG_TILEVIEW_INT_PTR:
		*(int *)ctrl->vals[nval].p = v;
		break;
	case RG_TILEVIEW_UINT_PTR:
		*(Uint *)ctrl->vals[nval].p = (Uint)v;
		break;
	default:
		AG_FatalError("cannot convert");
	}
}
static __inline__ void
RG_TileviewSetFloat(RG_TileviewCtrl *_Nonnull ctrl, int nval, float v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = v;
		break;
	case RG_TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = (double)v;
		break;
	case RG_TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = v;
		break;
	case RG_TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = (double)v;
		break;
	default:
		AG_FatalError("cannot convert");
	}
}
static __inline__ void
RG_TileviewSetDouble(RG_TileviewCtrl *_Nonnull ctrl, int nval, double v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = (float)v;
		break;
	case RG_TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = v;
		break;
	case RG_TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = (float)v;
		break;
	case RG_TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = v;
		break;
	default:
		AG_FatalError("cannot convert");
	}
}

#define RG_TileviewUint(ctrl,nval) (Uint)RG_TileviewInt((ctrl),(nval))
#define RG_TileviewSetUint(tv,nval,v) RG_TileviewSetInt((tv),(nval),(Uint)(v))
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_RG_TILEVIEW_H */
