/*	$Csoft: ucombo.h,v 1.1 2003/11/10 22:39:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_UCOMBO_H_
#define _AGAR_WIDGET_UCOMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

struct ucombo {
	struct widget wid;
	struct button *button;		/* Selection button */
	struct tlist *list;		/* Item list */
	struct window *win;		/* Pop-up window */
	int saved_w, saved_h;		/* Saved pop-up window geometry */
};

__BEGIN_DECLS
struct ucombo	*ucombo_new(void *);
void		 ucombo_init(struct ucombo *);
void		 ucombo_scale(void *, int, int);
void		 ucombo_destroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UCOMBO_H_ */
