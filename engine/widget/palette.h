/*	$Csoft: palette.h,v 1.3 2003/02/02 21:16:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PALETTE_H_
#define _AGAR_WIDGET_PALETTE_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

enum palette_channel {
	PALETTE_RED,
	PALETTE_GREEN,
	PALETTE_BLUE,
	PALETTE_ALPHA
};

struct palette {
	struct widget	wid;
	int		flags;
#define PALETTE_RGB		0x01	/* Manipulate the RGB triplet */
#define PALETTE_ALPHA		0x02	/* Manipulate the alpha channel */

	struct scrollbar	 *cur_sb;	/* Selected scrollbar */
	struct scrollbar	**bars;
	int			 nbars;
	SDL_Rect		  rpreview;

	struct {			/* Default binding */
		Uint32	color;
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct palette	*palette_new(struct region *, int, int, int);
extern DECLSPEC void		 palette_init(struct palette *, int, int, int);
extern DECLSPEC void	 	 palette_destroy(void *);
extern DECLSPEC void		 palette_draw(void *);
extern DECLSPEC void		 palette_scaled(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PALETTE_H_ */
