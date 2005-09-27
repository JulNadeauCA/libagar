/*	$Csoft: tileview.h,v 1.32 2005/09/22 02:30:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILEVIEW_H_
#define _AGAR_BG_TILEVIEW_H_

#include <engine/widget/widget.h>
#include <engine/widget/menu.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/box.h>

#include <engine/rg/tileset.h>
#include <engine/timeout.h>

#include "begin_code.h"

#define RG_TILEVIEW_MIN_W	32
#define RG_TILEVIEW_MIN_H	32

enum rg_tileview_ctrl_type {
	RG_TILEVIEW_POINT,			/* Point (x,y) */
	RG_TILEVIEW_RECTANGLE,		/* Rectangle (x,y,w,h) */
	RG_TILEVIEW_RDIMENSIONS,		/* Rectangle (w,h) */
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
	u_int ui;
	float f;
	double d;
	void *p;
};

struct rg_tileview_handle {
	int x, y;		/* Cached tile coords (set by draw routine) */
	int over;		/* Mouse overlap */
	int enable;		/* Mouse selection */
};

typedef struct rg_tileview_ctrl {
	enum rg_tileview_ctrl_type type;

	SDL_Color c, cIna, cEna, cOver, cHigh, cLow;
	Uint8	  a, aIna, aEna, aOver;

	enum tileview_val_type	*valtypes;		/* Entry types */
	union rg_tileview_val	*vals;			/* Values/pointers */
	u_int			nvals;
	struct rg_tileview_handle *handles;		/* User handles */
	u_int			  nhandles;

	VG *vg;					/* For RG_TILEVIEW_VERTEX */
	VG_Element *vge;

	AG_Event *motion;
	AG_Event *buttonup;
	AG_Event *buttondown;
	int xoffs, yoffs;

	TAILQ_ENTRY(rg_tileview_ctrl) ctrls;
} RG_TileviewCtrl;

typedef struct rg_tileview_tool_ops {
	const char *name;
	const char *desc;
	size_t len;
	int flags;
	int icon, cursor;

	void		(*init)(void *);
	void		(*destroy)(void *);
	AG_Window  *(*edit)(void *);
	void		(*selected)(void *);
	void		(*unselected)(void *);
} RG_TileviewToolOps;

typedef struct rg_tileview_bitmap_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*mousebuttondown)(void *, int, int, int);
	void (*mousebuttonup)(void *, int, int, int);
	void (*mousemotion)(void *, int, int, int, int);
} RG_TileviewBitmapToolOps;

typedef struct rg_tileview_sketch_tool_ops {
	struct rg_tileview_tool_ops ops;
	void (*mousebuttondown)(void *, RG_Sketch *, double, double, int);
	void (*mousebuttonup)(void *, RG_Sketch *, double, double, int);
	void (*mousemotion)(void *, RG_Sketch *, double, double, double,
	                    double);
	int (*mousewheel)(void *, RG_Sketch *, int);
	void (*keydown)(void *, RG_Sketch *, int, int);
	void (*keyup)(void *, RG_Sketch *, int, int);
} RG_TileviewSketchToolOps;

typedef struct rg_tileview_tool {
	const RG_TileviewToolOps *ops;
	struct rg_tileview *tv;
	int flags;
#define TILEVIEW_TILE_TOOL	0x01	/* Call in default edition mode */
#define TILEVIEW_FEATURE_TOOL	0x02	/* Call in feature edition mode */
#define TILEVIEW_SKETCH_TOOL	0x04	/* Call in vector edition mode */
#define TILEVIEW_PIXMAP_TOOL	0x08	/* Call in pixmap edition mode */
	AG_Window *win;
	TAILQ_ENTRY(rg_tileview_tool) tools;
} RG_TileviewTool;

enum rg_tileview_state {
	RG_TILEVIEW_TILE_EDIT,	/* Default edition mode */
	RG_TILEVIEW_FEATURE_EDIT,	/* A feature is being edited */
	RG_RG_TILEVIEW_SKETCH_EDIT,	/* A sketch is being edited inline */
	RG_TILEVIEW_PIXMAP_EDIT,	/* A pixmap is being edited inline */
	RG_TILEVIEW_ATTRIB_EDIT,	/* Node attributes are being edited */
	RG_TILEVIEW_LAYERS_EDIT	/* Node layers are being edited */
};

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
	SDL_Surface *scaled;		/* Scaled surface */
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
	
	AG_Menu *menu;		/* Popup menu */
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
		struct {
			RG_TileElement *tel;	/* Sketch element */
			RG_Sketch *sk;
			AG_Window *win;		/* Settings */
			RG_TileviewCtrl *ctrl;	/* Extent control */
			AG_Menu *menu;		/* Popup menu */
			AG_MenuItem *menu_item;	/* Popup menu item */
			AG_Window *menu_win;	/* Popup menu window */
		} sketch;
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
			int xorig, yorig;		/* Reference point
							   for restrictions */
		} pixmap;
		struct {
			RG_TileviewCtrl *geo_ctrl;	 /* Geometry control */
			RG_TileviewCtrl *orig_ctrl; /* Origin control */
		} tile;
		struct {
			int nx, ny;			/* Current position */
		} attrs;
	} sargs;
#define tv_feature sargs.feature
#define tv_sketch  sargs.sketch
#define tv_pixmap  sargs.pixmap
#define tv_tile	   sargs.tile
#define tv_attrs   sargs.attrs

	struct {
		Uint8 r, g, b, a;		/* Current color */
		Uint32 pc;			/* (for binding controls) */
	} c;
	TAILQ_HEAD(,rg_tileview_ctrl) ctrls;	/* Binding controls */
	TAILQ_HEAD(,rg_tileview_tool) tools;	/* Edition tools */
	RG_TileviewTool *cur_tool;		/* Current tool */
} RG_Tileview;

#define RG_TILEVIEW_TOOL(p) ((RG_TileviewTool *)p)
#define RG_TILEVIEW_SCALED_X(tv, x) (AGWIDGET(tv)->cx + (tv)->xoffs + (x)*(tv)->pxsz)
#define RG_TILEVIEW_SCALED_Y(tv, x) (AGWIDGET(tv)->cy + (tv)->yoffs + (y)*(tv)->pxsz)

__BEGIN_DECLS
RG_Tileview	*RG_TileviewNew(void *, RG_Tileset *, int);
RG_TileviewTool *RG_TileviewRegTool(RG_Tileview *, const void *);

void RG_TileviewInit(RG_Tileview *, RG_Tileset *, int);
void RG_TileviewDestroy(void *);
void RG_TileviewDraw(void *);
void RG_TileviewScale(void *, int, int);

void RG_TileviewSetTile(RG_Tileview *, RG_Tile *);
void RG_TileviewSetZoom(RG_Tileview *, int, int);
void RG_TileviewSetAutoRefresh(RG_Tileview *, int, int);

void RG_TileviewColor3i(RG_Tileview *, Uint8, Uint8, Uint8);
void RG_TileviewColor4i(RG_Tileview *, Uint8, Uint8, Uint8, Uint8);
void RG_TileviewSDLColor(RG_Tileview *, SDL_Color *, Uint8);
void RG_TileviewAlpha(RG_Tileview *, Uint8);

void RG_TileviewPixel2i(RG_Tileview *, int, int);
void RG_TileviewRect2(RG_Tileview *, int, int, int, int);
void RG_TileviewRect2o(RG_Tileview *, int, int, int, int);
void RG_TileviewCircle2o(RG_Tileview *, int, int, int);
void RG_TileviewHLine(RG_Tileview *, int, int, int);
void RG_TileviewVLine(RG_Tileview *, int, int, int);

__inline__ void RG_TileviewScaledPixel(RG_Tileview *, int, int, Uint8,
		                      Uint8, Uint8);

RG_TileviewCtrl *RG_TileviewAddCtrl(RG_Tileview *,
			                   enum rg_tileview_ctrl_type,
					   const char *, ...);
void		      RG_TileviewDelCtrl(RG_Tileview *,
				           RG_TileviewCtrl *);

__inline__ int	  RG_TileviewInt(RG_TileviewCtrl *, int);
__inline__ void	  RG_TileviewSetInt(RG_TileviewCtrl *, int, int);
__inline__ float  RG_TileviewFloat(RG_TileviewCtrl *, int);
__inline__ void	  RG_TileviewSetFloat(RG_TileviewCtrl *, int, float);
__inline__ double RG_TileviewDouble(RG_TileviewCtrl *, int);
__inline__ void	  RG_TileviewSetDouble(RG_TileviewCtrl *, int, double);

#define RG_TileviewUint(ctrl,nval)	(u_int)RG_TileviewInt((ctrl),(nval))
#define RG_TileviewSetUint(tv,nval,v)	RG_TileviewSetInt((tv),(nval),(u_int)(v))

void RG_TileviewSelectTool(RG_Tileview *, RG_TileviewTool *);
void RG_TileviewUnselectTool(RG_Tileview *);

void RG_TileviewGenericMenu(RG_Tileview *, AG_MenuItem *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */
