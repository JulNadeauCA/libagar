/*	$Csoft: tileview.h,v 1.1 2005/01/13 02:30:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_BG_TILEVIEW_H_
#define _AGAR_BG_TILEVIEW_H_

#include <engine/widget/widget.h>
#include <engine/rg/tileset.h>
#include <engine/timeout.h>

#include "begin_code.h"

struct tileview {
	struct widget wid;
	struct tileset *ts;
	struct tile *tile;
	int zoom;			/* Display zoom (%) */
	int xoffs, yoffs;		/* Display offset */
	SDL_Surface *scaled;		/* Scaled surface */
	int scrolling;
	int flags;
#define TILEVIEW_AUTOREGEN	0x01	/* Regenerate the tile periodically */
	struct timeout zoom_to;
	enum {
		TILEVIEW_TILE_EDIT,
		TILEVIEW_FEATURE_EDIT,
		TILEVIEW_SKETCH_EDIT,
		TILEVIEW_PIXMAP_EDIT
	} state;
	union {
		struct {
			struct window *edit_win;
		} feature;
	} sargs;
	int edit_mode;
};

__BEGIN_DECLS
struct tileview	*tileview_new(void *, struct tileset *, struct tile *, int);

void	 tileview_init(struct tileview *, struct tileset *, struct tile *, int);
void	 tileview_destroy(void *);
void	 tileview_draw(void *);
void	 tileview_scale(void *, int, int);
void	 tileview_resize(struct tileview *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_BG_TILEVIEW_H */
