/*	$Csoft: checkbox.h,v 1.11 2003/01/25 06:24:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

struct checkbox {
	struct widget	wid;
	char		*caption;	/* Label string */
	SDL_Surface	*label_s;	/* Label surface */
	int	 	 cbox_w;	/* Checkbox width */
	struct {
		int	 state;
	} def;
};

struct checkbox	*checkbox_new(struct region *, int, const char *, ...);
void		 checkbox_init(struct checkbox *, int, char *);
void		 checkbox_destroy(void *);
void		 checkbox_draw(void *);

#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
