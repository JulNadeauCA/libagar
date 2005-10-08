/*	$Csoft: combo.h,v 1.10 2005/05/24 08:12:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

typedef struct ag_combo {
	struct ag_widget wid;
	int flags;
#define AG_COMBO_POLL	  0x01		/* Polled list */
#define AG_COMBO_TREE	  0x02		/* Tree display */
#define AG_COMBO_ANY_TEXT 0x04		/* Accept text not matching an item */
	AG_Textbox *tbox;		/* Text input */
	AG_Button *button;		/* [...] button */
	AG_Tlist *list;			/* List of items */
	AG_Window *panel;
	int saved_h;			/* Saved panel height */
} AG_Combo;

__BEGIN_DECLS
AG_Combo *AG_ComboNew(void *, int, const char *, ...)
	         FORMAT_ATTRIBUTE(printf, 3, 4)
		 NONNULL_ATTRIBUTE(3);

void AG_ComboInit(AG_Combo *, const char *, int);
void AG_ComboScale(void *, int, int);
void AG_ComboDestroy(void *);
void combo_select(AG_Combo *, AG_TlistItem *);
AG_TlistItem *AG_ComboSelectPointer(AG_Combo *, void *);
AG_TlistItem *AG_ComboSelectText(AG_Combo *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_COMBO_H_ */
