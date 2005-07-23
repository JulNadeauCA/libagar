/*	$Csoft: tileview.h,v 1.28 2005/07/11 05:43:00 vedge Exp $	*/
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

#define TILEVIEW_MIN_W	32
#define TILEVIEW_MIN_H	32

enum tileview_ctrl_type {
	TILEVIEW_POINT,			/* Point (x,y) */
	TILEVIEW_RECTANGLE,		/* Rectangle (x,y,w,h) */
	TILEVIEW_RDIMENSIONS,		/* Rectangle (w,h) */
	TILEVIEW_CIRCLE,		/* Circle (x,y,r) */
	TILEVIEW_VERTEX,		/* Vg vertex (x,y) */
};

enum tileview_val_type {
	TILEVIEW_INT_VAL,
	TILEVIEW_INT_PTR,
	TILEVIEW_UINT_VAL,
	TILEVIEW_UINT_PTR,
	TILEVIEW_FLOAT_VAL,
	TILEVIEW_FLOAT_PTR,
	TILEVIEW_DOUBLE_VAL,
	TILEVIEW_DOUBLE_PTR,
};

union tileview_val {
	int i;
	u_int ui;
	float f;
	double d;
	void *p;
};

struct tileview_handle {
	int x, y;		/* Cached tile coords (set by draw routine) */
	int over;		/* Mouse overlap */
	int enable;		/* Mouse selection */
};

struct tileview_ctrl {
	enum tileview_ctrl_type type;

	SDL_Color c, cIna, cEna, cOver, cHigh, cLow;
	Uint8	  a, aIna, aEna, aOver;

	enum tileview_val_type	*valtypes;		/* Entry types */
	union tileview_val	*vals;			/* Values/pointers */
	u_int			nvals;
	struct tileview_handle	*handles;		/* User handles */
	u_int			nhandles;

	struct vg *vg;				/* For TILEVIEW_VERTEX */
	struct vg_element *vge;

	struct event *motion;
	struct event *buttonup;
	struct event *buttondown;
	int xoffs, yoffs;

	TAILQ_ENTRY(tileview_ctrl) ctrls;
};

struct tileview_tool_ops {
	const char *name;
	const char *desc;
	size_t len;
	int flags;
	int icon, cursor;

	void		(*init)(void *);
	void		(*destroy)(void *);
	struct window  *(*edit)(void *);
	void		(*selected)(void *);
	void		(*unselected)(void *);
};

struct tileview_bitmap_tool_ops {
	struct tileview_tool_ops ops;
	void (*mousebuttondown)(void *, int, int, int);
	void (*mousebuttonup)(void *, int, int, int);
	void (*mousemotion)(void *, int, int, int, int);
};

struct tileview_sketch_tool_ops {
	struct tileview_tool_ops ops;
	void (*mousebuttondown)(void *, struct sketch *, double, double, int);
	void (*mousebuttonup)(void *, struct sketch *, double, double, int);
	void (*mousemotion)(void *, struct sketch *, double, double, double,
	    double);
	int (*mousewheel)(void *, struct sketch *, int);
	void (*keydown)(void *, struct sketch *, int, int);
	void (*keyup)(void *, struct sketch *, int, int);
};

struct tileview_tool {
	const struct tileview_tool_ops *ops;
	struct tileview *tv;
	int flags;
#define TILEVIEW_TILE_TOOL	0x01	/* Call in default edition mode */
#define TILEVIEW_FEATURE_TOOL	0x02	/* Call in feature edition mode */
#define TILEVIEW_SKETCH_TOOL	0x04	/* Call in vector edition mode */
#define TILEVIEW_PIXMAP_TOOL	0x08	/* Call in pixmap edition mode */
	struct window *win;
	TAILQ_ENTRY(tileview_tool) tools;
};

enum tileview_state {
	TILEVIEW_TILE_EDIT,	/* Default edition mode */
	TILEVIEW_FEATURE_EDIT,	/* A feature is being edited */
	TILEVIEW_SKETCH_EDIT,	/* A sketch is being edited inline */
	TILEVIEW_PIXMAP_EDIT,	/* A pixmap is being edited inline */
	TILEVIEW_ATTRIB_EDIT	/* Node attributes are being edited */
};

struct tileview {
	struct widget wid;
	struct tileset *ts;
	struct tile *tile;
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
#define TILEVIEW_NO_SCROLLING	0x01	/* Disable right click scrolling */
#define TILEVIEW_HIDE_CONTROLS	0x02	/* Hide the current controls */
#define TILEVIEW_NO_TILING	0x04	/* Don't draw background tiling */
#define TILEVIEW_NO_EXTENT	0x08	/* Hide the extent rectangle */
#define TILEVIEW_NO_GRID	0x10	/* Hide the tile grid */
#define TILEVIEW_SET_ATTRIBS	0x20	/* Setting node attributes */

	struct timeout zoom_to;		/* Zoom timeout */
	struct timeout redraw_to;	/* Auto redraw timeout */

	int edit_attr;			/* Attribute being edited */
	int edit_mode;			/* Element is being edited */
	enum tileview_state state;
	struct box *tel_box;		/* Element-specific toolbar container */
	struct toolbar *tel_tbar;	/* Element-specific toolbar */
	
	struct AGMenu *menu;		/* Popup menu */
	struct AGMenuItem *menu_item;
	struct window *menu_win;

	union {
		struct {
			struct tile_element *tel;
			struct feature *ft;   
			struct window *win;

			struct AGMenu *menu;
			struct AGMenuItem *menu_item;
			struct window *menu_win;
		} feature;
		struct {
			struct tile_element *tel;
			struct sketch *sk;
			struct window *win;
			struct tileview_ctrl *ctrl;

			struct AGMenu *menu;
			struct AGMenuItem *menu_item;
			struct window *menu_win;
		} sketch;
		struct {
			struct tile_element *tel;
			struct pixmap *px;
			struct window *win;
			struct tileview_ctrl *ctrl;
			enum {
				TILEVIEW_PIXMAP_IDLE,
				TILEVIEW_PIXMAP_FREEHAND
			} state;

			struct AGMenu *menu;
			struct AGMenuItem *menu_item;
			struct window *menu_win;
		} pixmap;
		struct {
			struct tileview_ctrl *geo_ctrl;
			struct tileview_ctrl *orig_ctrl;
		} tile;
		struct {
			int nx, ny;
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
	TAILQ_HEAD(,tileview_ctrl) ctrls;	/* Binding controls */
	TAILQ_HEAD(,tileview_tool) tools;	/* Edition tools */
	struct tileview_tool *cur_tool;		/* Current tool */
};

#define TILEVIEW_TOOL(p) ((struct tileview_tool *)p)
#define TILEVIEW_SCALED_X(tv, x) (WIDGET(tv)->cx + (tv)->xoffs + (x)*(tv)->pxsz)
#define TILEVIEW_SCALED_Y(tv, x) (WIDGET(tv)->cy + (tv)->yoffs + (y)*(tv)->pxsz)

__BEGIN_DECLS
struct tileview	*tileview_new(void *, struct tileset *, int);
struct tileview_tool *tileview_reg_tool(struct tileview *, const void *);

void tileview_init(struct tileview *, struct tileset *, int);
void tileview_destroy(void *);
void tileview_draw(void *);
void tileview_scale(void *, int, int);

void tileview_set_tile(struct tileview *, struct tile *);
void tileview_set_zoom(struct tileview *, int, int);
void tileview_set_autoredraw(struct tileview *, int, int);

void tileview_color3i(struct tileview *, Uint8, Uint8, Uint8);
void tileview_color4i(struct tileview *, Uint8, Uint8, Uint8, Uint8);
void tileview_sdl_color(struct tileview *, SDL_Color *, Uint8);
void tileview_alpha(struct tileview *, Uint8);

void tileview_pixel2i(struct tileview *, int, int);
void tileview_rect2(struct tileview *, int, int, int, int);
void tileview_rect2o(struct tileview *, int, int, int, int);
void tileview_circle2o(struct tileview *, int, int, int);
void tileview_hline(struct tileview *, int, int, int);
void tileview_vline(struct tileview *, int, int, int);

__inline__ void tileview_scaled_pixel(struct tileview *, int, int, Uint8,
		                      Uint8, Uint8);

struct tileview_ctrl *tileview_insert_ctrl(struct tileview *,
			                   enum tileview_ctrl_type,
					   const char *, ...);
void		      tileview_remove_ctrl(struct tileview *,
				           struct tileview_ctrl *);

__inline__ int	  tileview_int(struct tileview_ctrl *, int);
__inline__ void	  tileview_set_int(struct tileview_ctrl *, int, int);
__inline__ float  tileview_float(struct tileview_ctrl *, int);
__inline__ void	  tileview_set_float(struct tileview_ctrl *, int, float);
__inline__ double tileview_double(struct tileview_ctrl *, int);
__inline__ void	  tileview_set_double(struct tileview_ctrl *, int, double);

#define tileview_uint(ctrl,nval)	(u_int)tileview_int((ctrl),(nval))
#define tileview_set_uint(tv,nval,v)	tileview_set_int((tv),(nval),(u_int)(v))

void tileview_select_tool(struct tileview *, struct tileview_tool *);
void tileview_unselect_tool(struct tileview *);

void tileview_generic_menu(struct tileview *, struct AGMenuItem *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */
