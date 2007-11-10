/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

typedef struct ag_radio_item {
	char text[128];
	int surface;
	SDLKey hotkey;
} AG_RadioItem;

typedef struct ag_radio {
	struct ag_widget wid;
	Uint flags;
#define AG_RADIO_HFILL	0x01
#define AG_RADIO_VFILL	0x02
#define AG_RADIO_EXPAND (AG_RADIO_HFILL|AG_RADIO_VFILL)
	int value;		/* Default value binding */
	AG_RadioItem *items;
	int nItems;
	int selitem;		/* Index of selected item */
	int max_w;		/* Width of widest label */
	int oversel;		/* Overlapping selection */
	int xPadding, yPadding;
	int xSpacing, ySpacing;
	int radius;
} AG_Radio;

__BEGIN_DECLS
extern const AG_WidgetOps agRadioOps;

AG_Radio *AG_RadioNew(void *, Uint, const char *[]);
void      AG_RadioItemsFromArray(AG_Radio *, const char **);
int       AG_RadioAddItem(AG_Radio *, const char *, ...);
int       AG_RadioAddItemHK(AG_Radio *, SDLKey, const char *, ...);
void      AG_RadioClearItems(AG_Radio *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_RADIO_H_ */
