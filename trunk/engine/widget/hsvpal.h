/*	$Csoft: hsvpal.h,v 1.8 2005/05/23 01:28:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_HSVPAL_H_
#define _AGAR_WIDGET_HSVPAL_H_

#include <engine/widget/scrollbar.h>
#include <engine/widget/menu.h>
#include <engine/widget/window.h>

#include "begin_code.h"

struct hsvpal {
	struct widget	   wid;
	int flags;
#define HSVPAL_PIXEL	0x01		/* Edit the pixel binding */ 
#define HSVPAL_DIRTY	0x02		/* Redraw the palette */

	float		   h, s, v, a;	/* Default bindings */
	Uint32		   pixel;
	SDL_Rect	   rpreview;	/* Color preview rectangle */
	SDL_Surface	  *surface;	/* Cached surface */
	int		   selcircle_r;	/* Radius of selection circles */
	struct {
		int x, y;		/* Origin for circle of hues */
		int rout, rin;		/* Radii of the circle of hues */
		int spacing;		/* Spacing between circle and rect */
		int width;		/* Width of circular band (rout-rin) */
		float dh;		/* Calculated optimal hue increment */
	} circle;
	struct {
		int x, y;		/* Coordinates of triangle */
		int w, h;		/* Dimensions of triangle */
	} triangle;
	enum {
		HSVPAL_SEL_NONE,
		HSVPAL_SEL_H,		/* Selecting hue */
		HSVPAL_SEL_SV,		/* Selecting saturation/value */
		HSVPAL_SEL_A		/* Selecting transparency value */
	} state;

	struct AGMenu *menu;
	struct AGMenuItem *menu_item;
	struct window *menu_win;
	Uint32 cTile;
};

__BEGIN_DECLS
struct hsvpal	*hsvpal_new(void *);
void		 hsvpal_init(struct hsvpal *);
void		 hsvpal_destroy(void *);
void		 hsvpal_scale(void *, int, int);
void		 hsvpal_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_HSVPAL_H_ */
