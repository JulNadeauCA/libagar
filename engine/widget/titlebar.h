/*	$Csoft: titlebar.h,v 1.2 2003/06/18 00:47:04 vedge Exp $	*/
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
struct titlebar	*titlebar_new(void *, int);

void	 titlebar_init(struct titlebar *, int);
void	 titlebar_draw(void *);
void	 titlebar_set_caption(struct titlebar *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
