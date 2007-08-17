/*	Public domain	*/

#ifndef _AGAR_WIDGET_COMBO_H_
#define _AGAR_WIDGET_COMBO_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/textbox.h>
#include <gui/button.h>
#include <gui/window.h>
#include <gui/tlist.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>
#endif

#include "begin_code.h"

typedef struct ag_combo {
	struct ag_widget wid;
	Uint flags;
#define AG_COMBO_POLL	  0x01		/* Polled list */
#define AG_COMBO_TREE	  0x02		/* Tree display */
#define AG_COMBO_ANY_TEXT 0x04		/* Accept text not matching an item */
#define AG_COMBO_HFILL	  0x08
#define AG_COMBO_VFILL	  0x10
#define AG_COMBO_FOCUS	  0x20
#define AG_COMBO_EXPAND	  (AG_COMBO_HFILL|AG_COMBO_VFILL)

	AG_Textbox *tbox;		/* Text input */
	AG_Button *button;		/* [...] button */
	AG_Tlist *list;			/* List of items */
	AG_Window *panel;
	int saved_w, saved_h;		/* Saved panel height */
} AG_Combo;

__BEGIN_DECLS
extern const AG_WidgetOps agComboOps;

AG_Combo *AG_ComboNew(void *, Uint, const char *);
void AG_ComboInit(AG_Combo *, Uint, const char *);
void AG_ComboDestroy(void *);
void AG_ComboSelect(AG_Combo *, AG_TlistItem *);
AG_TlistItem *AG_ComboSelectPointer(AG_Combo *, void *);
AG_TlistItem *AG_ComboSelectText(AG_Combo *, const char *);
void AG_ComboSetButtonText(AG_Combo *, const char *);
void AG_ComboSetButtonTextNODUP(AG_Combo *, char *);
void AG_ComboSetButtonSurface(AG_Combo *, SDL_Surface *);
void AG_ComboSetButtonSurfaceNODUP(AG_Combo *, SDL_Surface *);

void AG_ComboSizeRequest(void *, AG_SizeReq *);
int  AG_ComboSizeAllocate(void *, const AG_SizeAlloc *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_COMBO_H_ */
