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
#define AG_TITLEBAR_NO_CLOSE	0x01
#define AG_TITLEBAR_NO_MINIMIZE	0x02
#define AG_TITLEBAR_NO_MAXIMIZE	0x04
	int pressed;
	struct ag_window *win;		/* Back pointer to window */
	AG_Label *label;
	AG_Button *close_btn;
	AG_Button *minimize_btn;
	AG_Button *maximize_btn;
} AG_Titlebar;

__BEGIN_DECLS
extern AG_WidgetClass agTitlebarClass;
AG_Titlebar *AG_TitlebarNew(void *, Uint);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
