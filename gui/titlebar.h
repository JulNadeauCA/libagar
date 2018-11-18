/*	Public domain	*/

#ifndef _AGAR_WIDGET_TITLEBAR_H_
#define _AGAR_WIDGET_TITLEBAR_H_

#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>
#include <agar/gui/begin.h>

struct ag_window;

typedef struct ag_titlebar {
	AG_Box hb;
	Uint flags;
#define AG_TITLEBAR_NO_CLOSE	0x01	/* No close button */
#define AG_TITLEBAR_NO_MINIMIZE	0x02	/* No minimize button */
#define AG_TITLEBAR_NO_MAXIMIZE	0x04	/* No maximize button */
#define AG_TITLEBAR_PRESSED	0x08	/* Titlebar is pressed down */
#define AG_TITLEBAR_SAVED_FLAGS	(AG_TITLEBAR_NO_CLOSE|AG_TITLEBAR_NO_MINIMIZE|\
				 AG_TITLEBAR_NO_MAXIMIZE)

	struct ag_window *_Nullable win;	/* Back pointer to window */

	AG_Label  *_Nonnull label;		/* Titlebar text */
	AG_Button *_Nullable close_btn;		/* "Close" button */
	AG_Button *_Nullable minimize_btn;	/* Minimize button */
	AG_Button *_Nullable maximize_btn;	/* Maximize button */
} AG_Titlebar;

__BEGIN_DECLS
extern AG_WidgetClass agTitlebarClass;
AG_Titlebar *_Nonnull AG_TitlebarNew(void *_Nonnull, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
