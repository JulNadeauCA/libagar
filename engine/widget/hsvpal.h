/*	$Csoft: hsvpal.h,v 1.7 2005/01/23 11:49:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_HSVPAL_H_
#define _AGAR_WIDGET_HSVPAL_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

struct hsvpal {
	struct widget	   wid;
	float		   h, s, v;	/* Default bindings */
	SDL_PixelFormat	  *format;	/* Target pixel format */
	SDL_Rect	   rpreview;	/* Color preview rectangle */
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
};

__BEGIN_DECLS
struct hsvpal	*hsvpal_new(void *, SDL_PixelFormat *);
void		 hsvpal_init(struct hsvpal *, SDL_PixelFormat *);
void		 hsvpal_destroy(void *);
void		 hsvpal_scale(void *, int, int);
void		 hsvpal_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_HSVPAL_H_ */
