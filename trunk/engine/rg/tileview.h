/*	$Csoft: tileview.h,v 1.6 2005/02/11 04:50:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILEVIEW_H_
#define _AGAR_BG_TILEVIEW_H_

#include <engine/widget/widget.h>
#include <engine/rg/tileset.h>
#include <engine/timeout.h>

#include "begin_code.h"

#define TILEVIEW_MIN_W	32
#define TILEVIEW_MIN_H	32

enum tileview_ctrl_type {
	TILEVIEW_POINT,			/* Point (x,y) */
	TILEVIEW_RECTANGLE,		/* Rectangle (x,y,w,h) */
	TILEVIEW_CIRCLE			/* Circle (x,y,r) */
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

struct tileview_ctrl {
	enum tileview_ctrl_type type;
	Uint8 r, g, b, a;
	struct event *event;

	enum tileview_val_type	*valtypes;
	union tileview_val	*vals;
	unsigned int		nvals;

	TAILQ_ENTRY(tileview_ctrl) ctrls;
};

struct tileview {
	struct widget wid;
	struct tileset *ts;
	struct tile *tile;
	int zoom;			/* Display zoom (%) */
	int pxsz;			/* Scaled pixel size (pixels) */
	int xoffs, yoffs;		/* Display offset */
	int xms, yms;			/* Cursor coords in surface (pixels) */
	int xsub, ysub;			/* Cursor subpixel coords (v.pixels) */
	SDL_Surface *scaled;		/* Scaled surface */
	int scrolling;
	int flags;
#define TILEVIEW_AUTOREGEN	0x01	/* Regenerate the tile periodically */
#define TILEVIEW_PRESEL		0x02	/* Pre-selection mode (ctrl) */

	struct timeout zoom_to;		/* Zoom timeout */
	struct timeout redraw_to;	/* Auto redraw timeout */

	int edit_mode;
	enum {
		TILEVIEW_TILE_EDIT,	/* Default edition mode */
		TILEVIEW_FEATURE_EDIT,	/* A feature is being edited */
		TILEVIEW_SKETCH_EDIT,	/* A sketch is being edited inline */
		TILEVIEW_PIXMAP_EDIT	/* A pixmap is being edited inline */
	} state;
	union {
		struct {
			struct tile_element *tel;
			struct feature *ft;   
			struct window *win;
		} feature;
		struct {
			struct sketch *sk;
		} sketch;
		struct {
			struct tile_element *tel;
			struct pixmap *px;
			struct window *win;
			struct tileview_ctrl *ctrl;
		} pixmap;
	} sargs;
#define tv_feature sargs.feature
#define tv_sketch  sargs.sketch
#define tv_pixmap  sargs.pixmap

	struct {
		Uint8 r, g, b, a;		/* Current color */
		Uint32 pc;			/* (for binding controls) */
	} c;
	TAILQ_HEAD(,tileview_ctrl) ctrls;	/* Binding controls */
};

__BEGIN_DECLS
struct tileview	*tileview_new(void *, struct tileset *, struct tile *, int);

void tileview_init(struct tileview *, struct tileset *, struct tile *, int);
void tileview_destroy(void *);
void tileview_draw(void *);
void tileview_scale(void *, int, int);
void tileview_set_zoom(struct tileview *, int, int);
void tileview_set_autoredraw(struct tileview *, int, int);

void tileview_color3i(struct tileview *, Uint8, Uint8, Uint8);
void tileview_color4i(struct tileview *, Uint8, Uint8, Uint8, Uint8);
void tileview_alpha(struct tileview *, Uint8);

void tileview_pixel2i(struct tileview *, int, int);
void tileview_rect2(struct tileview *, int, int, int, int);
void tileview_rect2o(struct tileview *, int, int, int, int);
void tileview_circle2o(struct tileview *, int, int, int);
void tileview_handle(struct tileview *, int, int, int, int);

struct tileview_ctrl *tileview_insert_ctrl(struct tileview *,
			                   enum tileview_ctrl_type,
					   const char *, ...);
void		      tileview_remove_ctrl(struct tileview *,
				           struct tileview_ctrl *);

__inline__ int tileview_int(struct tileview_ctrl *, int);
#define tileview_uint(ctrl,nval) (u_int)tileview_int((ctrl),(nval))
__inline__ float tileview_float(struct tileview_ctrl *, int);
__inline__ double tileview_double(struct tileview_ctrl *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */
