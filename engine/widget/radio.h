/*	$Csoft: radio.h,v 1.9 2003/05/26 03:03:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct radio {
	struct widget	 wid;
	int		 value;		/* Default value binding */

	SDL_Surface	**labels;	/* Cached label surfaces */
	const char	**items;
	int		 nitems;
	int		 selitem;	/* Index of selected item */
	int		 max_w;		/* Width of widest label */
	int		 radius;
};

__BEGIN_DECLS
extern DECLSPEC struct radio	*radio_new(void *, const char *[]);
extern DECLSPEC void		 radio_init(struct radio *, const char *[]);
extern DECLSPEC void	 	 radio_draw(void *);
extern DECLSPEC void	 	 radio_scale(void *, int, int);
extern DECLSPEC void		 radio_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
