/*	$Csoft: window.h,v 1.77 2004/09/12 05:48:58 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_STYLE_H_
#define _AGAR_WIDGET_STYLE_H_
#include "begin_code.h"

struct style_colormod {
	const char *type;		/* Apply to widgets of this type */
	const char *color_id;		/* Name of color to edit */
	SDL_Color   color;		/* Color value */
};

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
		const SDL_Color *h_border;	/* Horizontal border */
		unsigned int	 h_border_w;
		const SDL_Color	*v_border;	/* Vertical border */
		unsigned int	 v_border_w;

		SDL_Color	 highlight;	/* Highlight color */
		SDL_Color	 bgcolor;	/* Solid bg color */
		SDL_Color	*bggradient;	/* Gradient bg colors */
		const char	*bgtexture;	/* Texture (or NULL) */

		enum {
			NOTCH_RESIZE_STYLE,	/* Notches in border */
			DIAGONAL_RESIZE_STYLE,	/* Icon in lower-right */
		} resize_ctrl_type;
	} win;

	/* Widget-related cosmetic overrides */
	struct {
		const struct style_colormod *colormods;
		const struct style_texmod   *texmods;
	} wid;

	void	(*fn)(void *);		/* Apply misc. customizations */
};

#include "close_code.h"
#endif /* _AGAR_WIDGET_STYLE_H_ */
