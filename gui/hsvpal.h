/*	Public domain	*/

#ifndef _AGAR_WIDGET_HSVPAL_H_
#define _AGAR_WIDGET_HSVPAL_H_

#ifdef _AGAR_INTERNAL
#include <gui/scrollbar.h>
#include <gui/menu.h>
#include <gui/window.h>
#else
#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/window.h>
#endif

#include "begin_code.h"

typedef struct ag_hsvpal {
	struct ag_widget wid;
	int flags;
#define AG_HSVPAL_PIXEL	0x01		/* Edit the pixel binding */ 
#define AG_HSVPAL_DIRTY	0x02		/* Redraw the palette */
#define AG_HSVPAL_HFILL 0x04
#define AG_HSVPAL_VFILL 0x08
#define AG_HSVPAL_EXPAND (AG_HSVPAL_HFILL|AG_HSVPAL_VFILL)

	float h, s, v, a;		/* Default bindings */
	Uint32 pixel;			/* Calculated pixel */
	SDL_Rect rAlpha;		/* Alpha selector rectangle */
	SDL_Surface *surface;		/* Cached surface */
	int selcircle_r;		/* Radius of selection circles */
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
		AG_HSVPAL_SEL_NONE,
		AG_HSVPAL_SEL_H,	/* Selecting hue */
		AG_HSVPAL_SEL_SV,	/* Selecting saturation/value */
		AG_HSVPAL_SEL_A		/* Selecting transparency value */
	} state;

	AG_Menu *menu;
	AG_MenuItem *menu_item;
	AG_Window *menu_win;
	Uint32 cTile;
} AG_HSVPal;

__BEGIN_DECLS
extern AG_WidgetClass agHSVPalClass;
AG_HSVPal *AG_HSVPalNew(void *, Uint);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_HSVPAL_H_ */
