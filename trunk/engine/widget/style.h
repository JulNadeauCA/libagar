/*	$Csoft: style.h,v 1.1 2004/09/16 04:06:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_STYLE_H_
#define _AGAR_WIDGET_STYLE_H_
#include "begin_code.h"

struct style_texmod {
	const char *type;		/* Apply to widgets of this type */
	const char *tex_id;		/* Texture to edit */
	enum {
		TEXTURE_REPLACE,	/* Replace the texture */
		TEXTURE_SRCALPHA,	/* Blend the texture (alpha=src) */
		TEXTURE_DSTALPHA	/* Blend the texture (alpha=dst) */
	} mode;
	const char *tex_name;		/* Name of new texture (or NULL) */
};

struct style {
	const char *name;

	/* Window-related cosmetic settings */
	struct {
		SDL_Color *bggradient;		/* Gradient bg colors */
		const char *bgtexture;		/* Texture (or NULL) */
		enum {
			NOTCH_RESIZE_STYLE,	/* Notches in border */
			DIAGONAL_RESIZE_STYLE,	/* Icon in lower-right */
		} resize_ctrl_type;
	} win;
	const struct style_texmod *texmods;
	void (*fn)(void *);			/* Apply misc. customizations */
};

#include "close_code.h"
#endif /* _AGAR_WIDGET_STYLE_H_ */
