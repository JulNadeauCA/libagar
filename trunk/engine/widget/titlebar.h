/*	$Csoft: titlebar.h,v 1.4 2004/09/25 01:57:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TITLEBAR_H_
#define _AGAR_WIDGET_TITLEBAR_H_

#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct titlebar {
	struct box hb;
	int flags;
#define TITLEBAR_NO_CLOSE	0x01
#define TITLEBAR_NO_MINIMIZE	0x02
#define TITLEBAR_NO_MAXIMIZE	0x04
	int pressed;
	struct window *win;		/* Back pointer to window */
	struct label *label;
	struct button *close_btn;
	struct button *minimize_btn;
	struct button *maximize_btn;
};

enum {
	TITLEBAR_CLOSE_ICON,
	TITLEBAR_MINIMIZE_ICON,
	TITLEBAR_MAXIMIZE_ICON,
};

__BEGIN_DECLS
struct titlebar	*titlebar_new(void *, int);

void	 titlebar_init(struct titlebar *, int);
void	 titlebar_draw(void *);
void	 titlebar_set_caption(struct titlebar *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TITLEBAR_H_ */
