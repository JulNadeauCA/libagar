/*	$Csoft: titlebar.h,v 1.5 2005/03/11 05:13:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TITLEBAR_H_
#define _AGAR_WIDGET_TITLEBAR_H_

#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct ag_window;

typedef struct ag_titlebar {
	AG_Box hb;
	int flags;
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

enum {
	AG_TITLEBAR_CLOSE_ICON,
	AG_TITLEBAR_MINIMIZE_ICON,
	AG_TITLEBAR_MAXIMIZE_ICON,
};

__BEGIN_DECLS
AG_Titlebar *AG_TitlebarNew(void *, int);
void	     AG_TitlebarInit(AG_Titlebar *, int);
void	     AG_TitlebarDraw(void *);
void	     AG_TitlebarSetCaption(AG_Titlebar *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
