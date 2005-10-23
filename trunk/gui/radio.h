/*	$Csoft: radio.h,v 1.13 2005/05/13 09:21:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <agar/gui/widget.h>

#include "begin_code.h"

typedef struct ag_radio {
	struct ag_widget wid;
	Uint flags;
#define AG_RADIO_WFILL	0x01
#define AG_RADIO_HFILL	0x02
#define AG_RADIO_EXPAND (AG_RADIO_WFILL|AG_RADIO_HFILL)
	int value;		/* Default value binding */
	int *labels;		/* Label surface IDs */
	const char **items;
	int nitems;
	int selitem;		/* Index of selected item */
	int max_w;		/* Width of widest label */
	int oversel;		/* Overlapping selection */
} AG_Radio;

__BEGIN_DECLS
AG_Radio *AG_RadioNew(void *, Uint, const char *[]);
void	  AG_RadioInit(AG_Radio *, Uint, const char *[]);
void	  AG_RadioDraw(void *);
void	  AG_RadioScale(void *, int, int);
void	  AG_RadioDestroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
