/*	$Csoft: radio.h,v 1.8 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct radio {
	struct widget	wid;
	SDL_Surface	**labels;
	const char	**items;
	int		 nitems;
	int		 selitem;	/* Index of selected item */
	int		 max_w;		/* Width of widest label */
	int		 radius;
	struct {
		int	value;
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct radio	*radio_new(struct region *, const char *[]);
extern DECLSPEC void		 radio_init(struct radio *, const char *[]);
extern DECLSPEC void	 	 radio_draw(void *);
extern DECLSPEC void		 radio_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
