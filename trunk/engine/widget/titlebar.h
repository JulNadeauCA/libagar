/*	$Csoft: label.h,v 1.19 2003/05/16 01:26:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TITLEBAR_H_
#define _AGAR_WIDGET_TITLEBAR_H_

#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct titlebar {
	struct box hb;

	int	flags;
	int	pressed;

	struct window	*win;		/* Back pointer to window */
	struct label	*label;		/* Caption label */
	struct button	*close_bu;	/* Closure button */
	struct button	*hide_bu;	/* Iconification button */
};

enum {
	TITLEBAR_CLOSE_ICON,
	TITLEBAR_HIDE_ICON
};

__BEGIN_DECLS
extern DECLSPEC struct titlebar	*titlebar_new(void *, int);
extern DECLSPEC void	 titlebar_init(struct titlebar *, int);
extern DECLSPEC void	 titlebar_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
