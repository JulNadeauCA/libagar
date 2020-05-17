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
	struct ag_widget wid;		/* AG_Widget -> AG_Combo */

	Uint flags;
#define AG_COMBO_POLL	  	0x01	/* Polled list */
#define AG_COMBO_ANY_TEXT 	0x04	/* Accept text not matching an item */
#define AG_COMBO_HFILL	  	0x08
#define AG_COMBO_VFILL	  	0x10
#define AG_COMBO_SCROLLTOSEL	0x40	/* Scroll to initial selection */
#define AG_COMBO_EXPAND	  (AG_COMBO_HFILL|AG_COMBO_VFILL)

	Uint32 _pad;
	AG_Textbox *_Nonnull  tbox;	/* Text input */
	AG_Button  *_Nonnull  button;	/* [...] button */
	AG_Tlist   *_Nonnull  list;	/* List of items */
	AG_Window  *_Nullable panel;	/* Expanded panel */

	int wSaved, hSaved;			/* Saved popup list geometry */
	int wPreList, hPreList;		/* Size hints */
} AG_Combo;

#define AGCOMBO(obj)            ((AG_Combo *)(obj))
#define AGCCOMBO(obj)           ((const AG_Combo *)(obj))
#define AG_COMBO_SELF()          AGCOMBO( AG_OBJECT(0,"AG_Widget:AG_Combo:*") )
#define AG_COMBO_PTR(n)          AGCOMBO( AG_OBJECT((n),"AG_Widget:AG_Combo:*") )
#define AG_COMBO_NAMED(n)        AGCOMBO( AG_OBJECT_NAMED((n),"AG_Widget:AG_Combo:*") )
#define AG_CONST_COMBO_SELF()   AGCCOMBO( AG_CONST_OBJECT(0,"AG_Widget:AG_Combo:*") )
#define AG_CONST_COMBO_PTR(n)   AGCCOMBO( AG_CONST_OBJECT((n),"AG_Widget:AG_Combo:*") )
#define AG_CONST_COMBO_NAMED(n) AGCCOMBO( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Combo:*") )

#define AG_COMBO_FOREACH(it, com) \
	AG_TLIST_FOREACH(it, (com)->list)
#define AG_COMBO_FOREACH_ITEM(p, com, it, t) \
	AG_TLIST_FOREACH_ITEM((p),(com)->list, it, t)

__BEGIN_DECLS
extern AG_WidgetClass agComboClass;

AG_Combo *_Nonnull AG_ComboNew(void *_Nullable, Uint, const char *_Nullable, ...)
		              FORMAT_ATTRIBUTE(printf,3,4);
AG_Combo *_Nonnull AG_ComboNewS(void *_Nullable, Uint, const char *_Nullable);

void AG_ComboSizeHint(AG_Combo *_Nonnull, const char *_Nonnull, int);
void AG_ComboSizeHintPixels(AG_Combo *_Nonnull, int, int);
void AG_ComboSelect(AG_Combo *_Nonnull, AG_TlistItem *_Nonnull);

AG_TlistItem *_Nullable AG_ComboSelectPointer(AG_Combo *_Nonnull, void *_Nullable);
AG_TlistItem *_Nullable AG_ComboSelectText(AG_Combo *_Nonnull, const char *_Nonnull);

void AG_ComboSetButtonText(AG_Combo *_Nonnull, const char *_Nonnull);
void AG_ComboSetButtonSurface(AG_Combo *_Nonnull, AG_Surface *_Nullable);
void AG_ComboSetButtonSurfaceNODUP(AG_Combo *_Nonnull, AG_Surface *_Nullable);
__END_DECLS

#ifdef AG_LEGACY
# define AG_COMBO_TREE 0
#endif

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_COMBO_H_ */
