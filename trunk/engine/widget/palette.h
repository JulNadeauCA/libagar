/*	$Csoft: palette.h,v 1.15 2002/11/19 05:06:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PALETTE_H_
#define _AGAR_WIDGET_PALETTE_H_

#include "scrollbar.h"

struct palette {
	struct widget	 wid;

	int	flags;
#define PALETTE_HEX_TRIPLET	0x01	/* Display the hex triplet */

	struct scrollbar r_sb;
	struct scrollbar g_sb;
	struct scrollbar b_sb;
	struct scrollbar a_sb;

	SDL_Rect rpreview;
};

struct palette	*palette_new(struct region *, int, int, int);
void		 palette_init(struct palette *, int, int, int);
void	 	 palette_destroy(void *);
void		 palette_draw(void *);
void		 palette_scaled(int, union evarg *);
void		 palette_set_color(struct palette *, Uint8, Uint8, Uint8,
		     Uint8);
void		 palette_get_color(struct palette *, Uint8 *, Uint8 *, Uint8 *,
		     Uint8 *);

#endif /* _AGAR_WIDGET_PALETTE_H_ */
