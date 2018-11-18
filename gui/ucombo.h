/*	Public domain	*/

#ifndef _AGAR_WIDGET_UCOMBO_H_
#define _AGAR_WIDGET_UCOMBO_H_

#include <agar/gui/widget.h>
#include <agar/gui/button.h>
#include <agar/gui/window.h>
#include <agar/gui/tlist.h>
#include <agar/gui/begin.h>

typedef struct ag_ucombo {
	struct ag_widget wid;		/* AG_Widget(3) -> AG_UCombo */
	Uint flags;
#define AG_UCOMBO_HFILL	  	0x01
#define AG_UCOMBO_VFILL	  	0x02
#define AG_UCOMBO_SCROLLTOSEL	0x40	/* Scroll to initial selection */
#define AG_UCOMBO_EXPAND  (AG_UCOMBO_HFILL|AG_UCOMBO_VFILL)

	AG_Button *_Nonnull  button;	/* Selection button */
	AG_Tlist  *_Nonnull  list;	/* Item list */
	AG_Window *_Nullable panel;	/* Expanded panel */

	int wSaved, hSaved;		/* Saved popup list geometry */
	int wPreList, hPreList;		/* Size hints */
} AG_UCombo;

__BEGIN_DECLS
extern AG_WidgetClass agUComboClass;

AG_UCombo *_Nonnull AG_UComboNew(void *_Nullable, Uint);
AG_UCombo *_Nonnull AG_UComboNewPolled(void *_Nullable, Uint, _Nonnull AG_EventFn,
                                       const char *_Nullable, ...);
void AG_UComboSizeHint(AG_UCombo *_Nonnull, const char *_Nullable, int);
void AG_UComboSizeHintPixels(AG_UCombo *_Nonnull, int,int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_UCOMBO_H_ */
