/*	Public domain	*/

#ifndef _AGAR_WIDGET_TITLEBAR_H_
#define _AGAR_WIDGET_TITLEBAR_H_

#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>
#include <agar/gui/begin.h>

struct ag_window;

typedef struct ag_titlebar {
	AG_Box hb;                      /* AG_Widget -> AG_Box -> AG_Titlebar */
	Uint flags;
#define AG_TITLEBAR_NO_CLOSE    0x01    /* No close button */
#define AG_TITLEBAR_NO_MINIMIZE 0x02    /* No minimize button */
#define AG_TITLEBAR_NO_MAXIMIZE 0x04    /* No maximize button */
#define AG_TITLEBAR_PRESSED     0x08    /* Titlebar is pressed down */
#define AG_TITLEBAR_NO_BUTTONS (AG_TITLEBAR_NO_CLOSE | \
                                AG_TITLEBAR_NO_MINIMIZE | \
                                AG_TITLEBAR_NO_MAXIMIZE)
#define AG_TITLEBAR_SAVED_FLAGS (AG_TITLEBAR_NO_BUTTONS)
	Uint32 _pad;
	struct ag_window *_Nullable win;	/* Back pointer to window */
	AG_Label *_Nonnull label;		/* Titlebar text */
} AG_Titlebar;

#define AGTITLEBAR(obj)            ((AG_Titlebar *)(obj))
#define AGCTITLEBAR(obj)           ((const AG_Titlebar *)(obj))
#define AG_TITLEBAR_SELF()          AGTITLEBAR( AG_OBJECT(0,"AG_Widget:AG_Box:AG_Titlebar:*") )
#define AG_TITLEBAR_PTR(n)          AGTITLEBAR( AG_OBJECT((n),"AG_Widget:AG_Box:AG_Titlebar:*") )
#define AG_TITLEBAR_NAMED(n)        AGTITLEBAR( AG_OBJECT_NAMED((n),"AG_Widget:AG_Box:AG_Titlebar:*") )
#define AG_CONST_TITLEBAR_SELF()   AGCTITLEBAR( AG_CONST_OBJECT(0,"AG_Widget:AG_Box:AG_Titlebar:*") )
#define AG_CONST_TITLEBAR_PTR(n)   AGCTITLEBAR( AG_CONST_OBJECT((n),"AG_Widget:AG_Box:AG_Titlebar:*") )
#define AG_CONST_TITLEBAR_NAMED(n) AGCTITLEBAR( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Box:AG_Titlebar:*") )

__BEGIN_DECLS
extern AG_WidgetClass agTitlebarClass;

AG_Titlebar *_Nonnull AG_TitlebarNew(void *_Nonnull, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
