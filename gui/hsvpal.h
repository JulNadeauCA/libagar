/*	Public domain	*/

#ifndef _AGAR_WIDGET_HSVPAL_H_
#define _AGAR_WIDGET_HSVPAL_H_

#include <agar/gui/scrollbar.h>
#include <agar/gui/menu.h>
#include <agar/gui/window.h>

#include <agar/gui/begin.h>

typedef struct ag_hsvpal {
	struct ag_widget wid;		/* AG_Widget -> AG_HSVPal */
	Uint flags;
#define AG_HSVPAL_PIXEL		0x01	/* Bound to a pixel/pixel format */
#define AG_HSVPAL_DIRTY		0x02	/* Redraw the palette */
#define AG_HSVPAL_HFILL 	0x04
#define AG_HSVPAL_VFILL 	0x08
#define AG_HSVPAL_NOALPHA	0x10	/* Disable alpha slider by default */
#define AG_HSVPAL_FORCE_NOALPHA	0x10	/* Disable alpha regardless of pixel-format */
#define AG_HSVPAL_NOPREVIEW	0x20	/* Disable color preview */
#define AG_HSVPAL_SHOW_RGB	0x40	/* Print RGB value */
#define AG_HSVPAL_SHOW_HSV	0x80	/* Print HSV value */
#define AG_HSVPAL_SHOW_RGB_HSV	0xc0	/* Print both RGB and HSV values */
#define AG_HSVPAL_EXPAND (AG_HSVPAL_HFILL|AG_HSVPAL_VFILL)

	float h, s, v, a;		/* Default bindings */
	Uint32 pixel;			/* Packed 32-bit pixel */
#if AG_MODEL == AG_LARGE
	Uint64 pixel64;			/* Packed 64-bit pixel */
#endif
	AG_Color color;			/* Native Agar color */
	AG_Rect rPrev;			/* Filled color preview area */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad1;
#endif
	AG_Surface *_Nullable surface;	/* Cached surface */
	int surfaceId;
	int selcircle_r;		/* Radius of selection circles */
	struct {
		int x, y;		/* Origin for circle of hues */
		int rOut, rIn;		/* Radii of the circle of hues */
		int spacing;		/* Spacing between circle and rect */
		int width;		/* Width of circular band (rout-rin) */
		float dh;		/* Calculated optimal hue increment */
	} circle;
	struct {
		int x, y;		/* Coordinates of triangle */
		int _pad2, h;		/* Dimensions of triangle */
	} triangle;
	enum {
		AG_HSVPAL_SEL_NONE,
		AG_HSVPAL_SEL_H,  /* Selecting hue */
		AG_HSVPAL_SEL_SV, /* Selecting saturation/value */
		AG_HSVPAL_SEL_A	  /* Selecting transparency value */
	} state;

	AG_Menu *_Nullable menu;        /* Popup menu (TODO use AG_PopupMenu) */
	AG_MenuItem *_Nullable menu_item;
	AG_Window *_Nullable menu_win;
	AG_Color cTile[2];		/* Tiling fill color (TODO use style) */
	AG_Timer toMove[4];             /* For 4-way keyboard navigation */
} AG_HSVPal;

#define AGHSVPAL(obj)            ((AG_HSVPal *)(obj))
#define AGCHSVPAL(obj)           ((const AG_HSVPal *)(obj))
#define AG_HSVPAL_SELF()          AGHSVPAL( AG_OBJECT(0,"AG_Widget:AG_HSVPal:*") )
#define AG_HSVPAL_PTR(n)          AGHSVPAL( AG_OBJECT((n),"AG_Widget:AG_HSVPal:*") )
#define AG_HSVPAL_NAMED(n)        AGHSVPAL( AG_OBJECT_NAMED((n),"AG_Widget:AG_HSVPal:*") )
#define AG_CONST_HSVPAL_SELF()   AGCHSVPAL( AG_CONST_OBJECT(0,"AG_Widget:AG_HSVPal:*") )
#define AG_CONST_HSVPAL_PTR(n)   AGCHSVPAL( AG_CONST_OBJECT((n),"AG_Widget:AG_HSVPal:*") )
#define AG_CONST_HSVPAL_NAMED(n) AGCHSVPAL( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_HSVPal:*") )

__BEGIN_DECLS
extern AG_WidgetClass agHSVPalClass;

AG_HSVPal *_Nonnull AG_HSVPalNew(void *_Nullable, Uint);
void                AG_HSVPal_UpdateHue(AG_HSVPal *_Nonnull, int, int);
void                AG_HSVPal_UpdateSV(AG_HSVPal *_Nonnull, int, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_HSVPAL_H_ */
