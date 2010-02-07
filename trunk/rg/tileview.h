/*	Public domain	*/

#ifndef _AGAR_RG_TILEVIEW_H_
#define _AGAR_RG_TILEVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/menu.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/box.h>
#include <agar/gui/iconmgr.h>

#include <agar/rg/tileset.h>

#include <agar/rg/begin.h>

#define RG_TILEVIEW_MIN_W	32
#define RG_TILEVIEW_MIN_H	32

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
	void *p;
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

	enum tileview_val_type	*valtypes;		/* Entry types */
	union rg_tileview_val	*vals;			/* Values/pointers */
	Uint			nvals;
	struct rg_tileview_handle *handles;		/* User handles */
	Uint			  nhandles;
#if 0
	VG *vg;					/* For RG_TILEVIEW_VERTEX */
	VG_Element *vge;
#endif
	AG_Event *motion;
	AG_Event *buttonup;
	AG_Event *buttondown;
	int xoffs, yoffs;

	AG_TAILQ_ENTRY(rg_tileview_ctrl) ctrls;
} RG_TileviewCtrl;

typedef struct rg_tileview_tool_ops {
	const char *name;     /* Name of tool */
	const char *desc;     /* Tool description */
	size_t len;           /* Size of structure */
	int flags;
	AG_StaticIcon *icon;  /* Specific icon (or NULL) */
	int cursor;           /* Specific cursor (or -1) */
	void       (*init)(void *);
	void       (*destroy)(void *);
	AG_Window *(*edit)(void *);
	void       (*selected)(void *);
	void       (*unselected)(void *);
} RG_TileviewToolOps;

typedef struct rg_tileview_bitmap_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*mousebuttondown)(void *, int, int, int);
	void (*mousebuttonup)(void *, int, int, int);
	void (*mousemotion)(void *, int, int, int, int);
} RG_TileviewBitmapToolOps;

#if 0
typedef struct rg_tileview_sketch_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*mousebuttondown)(void *, RG_Sketch *, float, float, int);
	void (*mousebuttonup)(void *, RG_Sketch *, float, float, int);
	void (*mousemotion)(void *, RG_Sketch *, float, float, float, float);
	int (*mousewheel)(void *, RG_Sketch *, int);
	void (*keydown)(void *, RG_Sketch *, int, int);
	void (*keyup)(void *, RG_Sketch *, int, int);
} RG_TileviewSketchToolOps;
#endif

typedef struct rg_tileview_tool {
	const RG_TileviewToolOps *ops;
	struct rg_tileview *tv;
	int flags;
#define TILEVIEW_TILE_TOOL	0x01	/* Call in default edition mode */
#define TILEVIEW_FEATURE_TOOL	0x02	/* Call in feature edition mode */
#define TILEVIEW_SKETCH_TOOL	0x04	/* Call in vector edition mode */
#define TILEVIEW_PIXMAP_TOOL	0x08	/* Call in pixmap edition mode */
	AG_Window *win;
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
	RG_Tileset *ts;
	RG_Tile *tile;
	int zoom;			/* Display zoom (%) */
	int pxsz;			/* Scaled pixel size (pixels) */
	int pxlen;			/* Scaled pixel size (bytes) */
	int xoffs, yoffs;		/* Display offset */
	int xms, yms;			/* Cursor coords in surface (pixels) */
	int xsub, ysub;			/* Cursor subpixel coords (v.pixels) */
	int xorig, yorig;		/* Origin used when moving controls */
	AG_Surface *scaled;		/* Scaled surface */
	int scrolling;
	int flags;
#define RG_TILEVIEW_NO_SCROLLING 0x01	/* Disable right click scrolling */
#define RG_TILEVIEW_HIDE_CONTROLS 0x02	/* Hide the current controls */
#define RG_TILEVIEW_NO_TILING	0x04	/* Don't draw background tiling */
#define RG_TILEVIEW_NO_EXTENT	0x08	/* Hide the extent rectangle */
#define RG_TILEVIEW_NO_GRID	0x10	/* Hide the tile grid */
#define RG_TILEVIEW_SET_ATTRIBS	0x20	/* Setting node attributes */
#define RG_TILEVIEW_READONLY	0x40

	AG_Timeout zoom_to;		/* Zoom timeout */
	AG_Timeout redraw_to;		/* Auto redraw timeout */

	int edit_attr;			/* Attribute being edited */
	int edit_mode;			/* Element is being edited */
	enum rg_tileview_state state;
	AG_Box *tel_box;		/* Element-specific toolbar container */
	AG_Toolbar *tel_tbar;		/* Element-specific toolbar */
	
	AG_Menu *menu;			/* Popup menu */
	AG_MenuItem *menu_item;
	AG_Window *menu_win;

	union {
		struct {
			RG_TileElement *tel;	/* Feature element */
			RG_Feature *ft;  
			AG_Window *win;		/* Settings */
			AG_Menu *menu;		/* Popup menu */
			AG_MenuItem *menu_item;	/* Popup menu item */
			AG_Window *menu_win;	/* Popup menu window */
		} feature;
#if 0
		struct {
			RG_TileElement *tel;	/* Sketch element */
			RG_Sketch *sk;
			AG_Window *win;		/* Settings */
			RG_TileviewCtrl *ctrl;	/* Extent control */
			AG_Menu *menu;		/* Popup menu */
			AG_MenuItem *menu_item;	/* Popup menu item */
			AG_Window *menu_win;	/* Popup menu window */
		} sketch;
#endif
		struct {
			RG_TileElement *tel;	/* Pixmap element */
			RG_Pixmap *px;
			AG_Window *win;		/* Settings */
			RG_TileviewCtrl *ctrl;	/* Extent control */
			enum {
				RG_TVPIXMAP_IDLE,
				RG_TVPIXMAP_FREEHAND,
				RG_TVPIXMAP_ORTHOGONAL,
				RG_TVPIXMAP_VERTICAL,
				RG_TVPIXMAP_HORIZONTAL,
				RG_TVPIXMAP_DIAGONAL
			} state;
			AG_Menu *menu;		/* Popup menu */
			AG_MenuItem *menu_item;	/* Popup menu item */
			AG_Window *menu_win;	/* Popup menu window */
			int xorig, yorig;	/* Ref pt for restrictions */
		} pixmap;
		struct {
			RG_TileviewCtrl *geo_ctrl;	/* Geometry control */
			RG_TileviewCtrl *orig_ctrl;	/* Origin control */
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
	RG_TileviewTool *cur_tool;		/* Current tool */
	struct ag_text_cache *tCache;		/* For "status" line */
} RG_Tileview;

#define RG_TILEVIEW_TOOL(p) ((RG_TileviewTool *)p)
#define RG_TILEVIEW_SCALED_X(tv, x) ((tv)->xoffs + (x)*(tv)->pxsz)
#define RG_TILEVIEW_SCALED_Y(tv, x) ((tv)->yoffs + (y)*(tv)->pxsz)

__BEGIN_DECLS
extern AG_WidgetClass rgTileviewClass;

RG_Tileview	*RG_TileviewNew(void *, RG_Tileset *, Uint);
RG_TileviewTool *RG_TileviewRegTool(RG_Tileview *, const void *);

void RG_TileviewSetTile(RG_Tileview *, RG_Tile *);
void RG_TileviewSetZoom(RG_Tileview *, int, int);
void RG_TileviewSetAutoRefresh(RG_Tileview *, int, int);

void RG_TileviewColor3i(RG_Tileview *, Uint8, Uint8, Uint8);
void RG_TileviewColor4i(RG_Tileview *, Uint8, Uint8, Uint8, Uint8);
void RG_TileviewColor(RG_Tileview *, AG_Color *, Uint8);
void RG_TileviewAlpha(RG_Tileview *, Uint8);

void RG_TileviewPixel2i(RG_Tileview *, int, int);
void RG_TileviewRect2(RG_Tileview *, int, int, int, int);
void RG_TileviewRect2o(RG_Tileview *, int, int, int, int);
void RG_TileviewCircle2o(RG_Tileview *, int, int, int);
void RG_TileviewHLine(RG_Tileview *, int, int, int);
void RG_TileviewVLine(RG_Tileview *, int, int, int);

RG_TileviewCtrl  *RG_TileviewAddCtrl(RG_Tileview *, enum rg_tileview_ctrl_type,
			             const char *, ...);
void		  RG_TileviewDelCtrl(RG_Tileview *, RG_TileviewCtrl *);

void RG_TileviewSelectTool(RG_Tileview *, RG_TileviewTool *);
void RG_TileviewUnselectTool(RG_Tileview *);
void RG_TileviewGenericMenu(RG_Tileview *, AG_MenuItem *);

/* Put (scaled) pixel on the tileview's cache surface. */
static __inline__ void
RG_TileviewScaledPixel(RG_Tileview *tv, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	int sx = x*tv->pxsz;
	int sy = y*tv->pxsz;
	Uint32 pixel;
	Uint8 *dst;

	if (sx < 0 || sy < 0 || sx >= tv->scaled->w || sy >= tv->scaled->h) {
		return;
	}
	pixel = AG_MapPixelRGB(tv->scaled->format, r,g,b);
	if (tv->pxsz == 1) {
		dst = (Uint8 *)tv->scaled->pixels + y*tv->scaled->pitch +
		    x*tv->scaled->format->BytesPerPixel;
		*(Uint32 *)dst = pixel;
	} else {
		int px, py;
		
		dst = (Uint8 *)tv->scaled->pixels +
		    sy*tv->scaled->pitch +
		    sx*tv->scaled->format->BytesPerPixel;
		for (py = 0; py < tv->pxsz; py++) {
			for (px = 0; px < tv->pxsz; px++) {
				*(Uint32 *)dst = pixel;
				dst += tv->scaled->format->BytesPerPixel;
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
RG_TileviewInt(RG_TileviewCtrl *ctrl, int nval)
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
RG_TileviewFloat(RG_TileviewCtrl *ctrl, int nval)
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
RG_TileviewDouble(RG_TileviewCtrl *ctrl, int nval)
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
RG_TileviewSetInt(RG_TileviewCtrl *ctrl, int nval, int v)
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
RG_TileviewSetFloat(RG_TileviewCtrl *ctrl, int nval, float v)
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
RG_TileviewSetDouble(RG_TileviewCtrl *ctrl, int nval, double v)
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

#include <agar/rg/close.h>
#endif /* _AGAR_RG_TILEVIEW_H */
