/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

typedef struct ag_radio_item {
	char text[128];
	int surface;
	AG_KeySym hotkey;
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
	int radius;		/* Control radius in pixels */
	int itemHeight;
	AG_Rect r;		/* View area */
} AG_Radio;

__BEGIN_DECLS
extern AG_WidgetClass agRadioClass;

AG_Radio *AG_RadioNew(void *, Uint, const char *[]);
AG_Radio *AG_RadioNewFn(void *, Uint, const char **, AG_EventFn,
                        const char *, ...);
AG_Radio *AG_RadioNewInt(void *, Uint, const char **, int *);
AG_Radio *AG_RadioNewUint(void *, Uint, const char **, Uint *);
void      AG_RadioItemsFromArray(AG_Radio *, const char **);
int       AG_RadioAddItem(AG_Radio *, const char *, ...)
                          FORMAT_ATTRIBUTE(printf,2,3)
			  NONNULL_ATTRIBUTE(2);
int       AG_RadioAddItemS(AG_Radio *, const char *);
int       AG_RadioAddItemHK(AG_Radio *, AG_KeySym, const char *, ...)
                            FORMAT_ATTRIBUTE(printf,3,4)
			    NONNULL_ATTRIBUTE(3);
int       AG_RadioAddItemHKS(AG_Radio *, AG_KeySym, const char *);
void      AG_RadioClearItems(AG_Radio *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_RADIO_H_ */
