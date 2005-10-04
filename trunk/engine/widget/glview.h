/*	$Csoft: button.h,v 1.30 2004/09/12 05:51:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GLVIEW_H_
#define _AGAR_WIDGET_GLVIEW_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_glview {
	struct ag_widget wid;
	u_int flags;
#define AG_GLVIEW_WFILL		0x01
#define AG_GLVIEW_HFILL		0x02
	AG_Event *draw_ev;
	AG_Event *reshape_ev;
} AG_GLView;

__BEGIN_DECLS
AG_GLView *AG_GLViewNew(void *, u_int);
void	   AG_GLViewInit(AG_GLView *, u_int);
void	   AG_GLViewDestroy(void *);
void	   AG_GLViewDraw(void *);
void	   AG_GLViewScale(void *, int, int);
void	   AG_GLViewDrawFn(AG_GLView *, void (*)(int, union evarg *),
		           const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_GLVIEW_H_ */
