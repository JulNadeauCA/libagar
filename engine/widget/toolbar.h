/*	$Csoft: toolbar.h,v 1.6 2003/11/10 22:41:12 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>

#include "begin_code.h"

enum toolbar_type {
	TOOLBAR_HORIZ,
	TOOLBAR_VERT
};

struct toolbar {
	struct box box;
	enum toolbar_type type;
};

__BEGIN_DECLS
struct toolbar	*toolbar_new(void *, enum toolbar_type);
void		 toolbar_init(struct toolbar *, enum toolbar_type);
void		 toolbar_scale(void *, int, int);
void	 	 toolbar_destroy(void *);
struct button	*toolbar_add_button(struct toolbar *, SDL_Surface *, int, int,
		                    void (*)(int, union evarg *), const char *,
				    ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
