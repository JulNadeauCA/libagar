/*	$Csoft: tileview.h,v 1.5 2005/02/05 06:06:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILEVIEW_H_
#define _AGAR_BG_TILEVIEW_H_

#include <engine/widget/widget.h>
#include <engine/rg/tileset.h>
#include <engine/timeout.h>

#include "begin_code.h"

#define TILEVIEW_MIN_W	32
#define TILEVIEW_MIN_H	32

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

	/* Edition state */
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
		} pixmap;
	} sargs;
#define tv_feature sargs.feature
#define tv_sketch  sargs.sketch
#define tv_pixmap  sargs.pixmap
};

__BEGIN_DECLS
struct tileview	*tileview_new(void *, struct tileset *, struct tile *, int);

void	 tileview_init(struct tileview *, struct tileset *, struct tile *, int);
void	 tileview_destroy(void *);
void	 tileview_draw(void *);
void	 tileview_scale(void *, int, int);
void	 tileview_set_zoom(struct tileview *, int, int);
void	 tileview_set_autoredraw(struct tileview *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */
