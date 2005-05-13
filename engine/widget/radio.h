/*	$Csoft: radio.h,v 1.12 2005/02/19 06:52:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct radio {
	struct widget	 wid;
	int		 value;		/* Default value binding */

	int		*labels;	/* Label surface IDs */
	const char	**items;
	int		 nitems;
	int		 selitem;	/* Index of selected item */
	int		 max_w;		/* Width of widest label */
	int		 oversel;	/* Overlapping selection */
};

__BEGIN_DECLS
struct radio	*radio_new(void *, const char *[]);
void		 radio_init(struct radio *, const char *[]);
void	 	 radio_draw(void *);
void	 	 radio_scale(void *, int, int);
void		 radio_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
