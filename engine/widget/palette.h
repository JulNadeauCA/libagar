/*	$Csoft: palette.h,v 1.1 2002/12/21 10:25:57 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PALETTE_H_
#define _AGAR_WIDGET_PALETTE_H_

#include "scrollbar.h"

enum palette_channel {
	PALETTE_RED,
	PALETTE_GREEN,
	PALETTE_BLUE,
	PALETTE_ALPHA
};

struct palette {
	struct widget	 wid;

	int	flags;
#define PALETTE_RGB		0x01	/* Manipulate the RGB triplet */
#define PALETTE_ALPHA		0x02	/* Manipulate the alpha channel */

	struct scrollbar	 *cur_sb;	/* Selected scrollbar */
	struct scrollbar	**bars;
	int			 nbars;

	SDL_Rect	rpreview;

	struct {			/* Default binding */
		Uint32	color;
	} def;
};

struct palette	*palette_new(struct region *, int, int, int);
void		 palette_init(struct palette *, int, int, int);
void	 	 palette_destroy(void *);
void		 palette_draw(void *);
void		 palette_scaled(int, union evarg *);

#endif /* _AGAR_WIDGET_PALETTE_H_ */
