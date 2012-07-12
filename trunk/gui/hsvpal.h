/*	Public domain	*/

#ifndef _AGAR_WIDGET_HSVPAL_H_
#define _AGAR_WIDGET_HSVPAL_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/window.h>

#include <agar/gui/begin.h>

typedef struct ag_hsvpal {
	struct ag_widget wid;
	Uint flags;
#define AG_HSVPAL_PIXEL		0x01	/* Edit the pixel binding */ 
#define AG_HSVPAL_DIRTY		0x02	/* Redraw the palette */
#define AG_HSVPAL_HFILL 	0x04
#define AG_HSVPAL_VFILL 	0x08
#define AG_HSVPAL_NOALPHA	0x10	/* Disable alpha slider by default */
#define AG_HSVPAL_FORCE_NOALPHA	0x10	/* Disable alpha regardless of pixel-format */
#define AG_HSVPAL_NOPREVIEW	0x20	/* Disable color preview */
#define AG_HSVPAL_SHOW_RGB	0x40	/* Print RGB value */
#define AG_HSVPAL_SHOW_HSV	0x80	/* Print HSV value */
#define AG_HSVPAL_EXPAND (AG_HSVPAL_HFILL|AG_HSVPAL_VFILL)

	float h, s, v, a;		/* Default bindings */
	Uint32 pixel;			/* Calculated pixel */
	AG_Color color;			/* Calculated color */
	AG_Rect rAlpha;			/* Alpha selector rectangle */
	AG_Surface *surface;		/* Cached surface */
	int surfaceId;
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
	AG_Color cTile;
} AG_HSVPal;

__BEGIN_DECLS
extern AG_WidgetClass agHSVPalClass;
AG_HSVPal *AG_HSVPalNew(void *, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_HSVPAL_H_ */
