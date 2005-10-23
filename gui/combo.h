/*	$Csoft: combo.h,v 1.10 2005/05/24 08:12:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>

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
AG_Combo *AG_ComboNew(void *, Uint, const char *);
void AG_ComboInit(AG_Combo *, Uint, const char *);
void AG_ComboScale(void *, int, int);
void AG_ComboDestroy(void *);
void AG_ComboSelect(AG_Combo *, AG_TlistItem *);
AG_TlistItem *AG_ComboSelectPointer(AG_Combo *, void *);
AG_TlistItem *AG_ComboSelectText(AG_Combo *, const char *);
void AG_ComboSetButtonText(AG_Combo *, const char *);
void AG_ComboSetButtonSurface(AG_Combo *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_COMBO_H_ */
