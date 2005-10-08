/*	$Csoft: radio.h,v 1.13 2005/05/13 09:21:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_radio {
	struct ag_widget wid;
	int value;		/* Default value binding */
	int *labels;		/* Label surface IDs */
	const char **items;
	int nitems;
	int selitem;		/* Index of selected item */
	int max_w;		/* Width of widest label */
	int oversel;		/* Overlapping selection */
} AG_Radio;

__BEGIN_DECLS
AG_Radio *AG_RadioNew(void *, const char *[]);
void	  AG_RadioInit(AG_Radio *, const char *[]);
void	  AG_RadioDraw(void *);
void	  AG_RadioScale(void *, int, int);
void	  AG_RadioDestroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
