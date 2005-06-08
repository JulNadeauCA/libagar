/*	$Csoft: style.h,v 1.2 2005/03/09 06:39:21 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_STYLE_H_
#define _AGAR_WIDGET_STYLE_H_
#include "begin_code.h"

typedef struct ag_widget_style_mod {
	const char *name;
	struct {
		SDL_Color *bggradient;		/* Gradient bg colors */
		const char *bgtexture;		/* Texture (or NULL) */
		enum {
			AG_NOTCH_RESIZE_STYLE,	  /* Notches on border */
			AG_DIAGONAL_RESIZE_STYLE, /* Icon in lower-right */
		} resize_ctrl_type;
	} win;
	void (*fn)(void *);			/* Apply misc. customizations */
} AG_WidgetStyleMod;

#include "close_code.h"
#endif /* _AGAR_WIDGET_STYLE_H_ */
