/*	$Csoft: toolbar.h,v 1.3 2004/04/13 01:21:46 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>

#include "begin_code.h"

#define TOOLBAR_MAX_ROWS	8

enum toolbar_type {
	TOOLBAR_HORIZ,
	TOOLBAR_VERT
};

struct toolbar_button {
	int row;				/* Assigned row */
	struct button *bu;
};

struct toolbar {
	struct box box;
	struct box *rows[TOOLBAR_MAX_ROWS];
	struct toolbar_button *buttons;
	enum toolbar_type type;
	int nrows;
};

__BEGIN_DECLS
struct toolbar	*toolbar_new(void *, enum toolbar_type, int);
void		 toolbar_init(struct toolbar *, enum toolbar_type, int);
void		 toolbar_scale(void *, int, int);
void	 	 toolbar_destroy(void *);
struct button	*toolbar_add_button(struct toolbar *, int, SDL_Surface *, int,
		                    int, void (*)(int, union evarg *),
				    const char *, ...);
void		 toolbar_select_unique(struct toolbar *, struct button *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
