/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>

#include <agar/gui/begin.h>

typedef struct ag_combo {
	struct ag_widget wid;
	Uint flags;
#define AG_COMBO_POLL	  	0x01	/* Polled list */
#define AG_COMBO_TREE	  	0x02	/* Tree display */
#define AG_COMBO_ANY_TEXT 	0x04	/* Accept text not matching an item */
#define AG_COMBO_HFILL	  	0x08
#define AG_COMBO_VFILL	  	0x10
#define AG_COMBO_SCROLLTOSEL	0x40	/* Scroll to initial selection */
#define AG_COMBO_EXPAND	  (AG_COMBO_HFILL|AG_COMBO_VFILL)

	AG_Textbox *tbox;		/* Text input */
	AG_Button *button;		/* [...] button */
	AG_Tlist *list;			/* List of items */
	AG_Window *panel;
	int wSaved, hSaved;		/* Saved popup list geometry */
	int wPreList, hPreList;		/* Size hints */
} AG_Combo;

#define AG_COMBO_FOREACH(it, com) \
	AG_TLIST_FOREACH(it, (com)->list)
#define AG_COMBO_FOREACH_ITEM(p, com, it, t) \
	AG_TLIST_FOREACH_ITEM((p),(com)->list, it, t)

__BEGIN_DECLS
extern AG_WidgetClass agComboClass;

AG_Combo *AG_ComboNew(void *, Uint, const char *, ...)
		      FORMAT_ATTRIBUTE(printf, 3, 4);
AG_Combo *AG_ComboNewS(void *, Uint, const char *);

void AG_ComboSizeHint(AG_Combo *, const char *, int);
void AG_ComboSizeHintPixels(AG_Combo *, int, int);
void AG_ComboSelect(AG_Combo *, AG_TlistItem *);
AG_TlistItem *AG_ComboSelectPointer(AG_Combo *, void *);
AG_TlistItem *AG_ComboSelectText(AG_Combo *, const char *);
void AG_ComboSetButtonText(AG_Combo *, const char *);
void AG_ComboSetButtonSurface(AG_Combo *, AG_Surface *);
void AG_ComboSetButtonSurfaceNODUP(AG_Combo *, AG_Surface *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_COMBO_H_ */
