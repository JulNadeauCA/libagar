/*	$Csoft: checkbox.h,v 1.12 2003/02/02 21:16:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct checkbox {
	struct widget	wid;
	char		*caption;	/* Label string */
	SDL_Surface	*label_s;	/* Label surface */
	int	 	 cbox_w;	/* Checkbox width */
	struct {
		int	 state;
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct checkbox	*checkbox_new(struct region *, int,
				              const char *, ...);
extern DECLSPEC void		 checkbox_init(struct checkbox *, int, char *);
extern DECLSPEC void		 checkbox_destroy(void *);
extern DECLSPEC void		 checkbox_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
