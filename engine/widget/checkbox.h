/*	$Csoft: checkbox.h,v 1.16 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define CHECKBOX_CAPTION_MAX	64

struct checkbox {
	struct widget wid;

	int		 state;		/* 1=pressed, 0=released */
	SDL_Surface	*label_su;	/* Text label */
	int		 label_id;
};

__BEGIN_DECLS
struct checkbox	*checkbox_new(void *, const char *, ...);
void		 checkbox_init(struct checkbox *, char *);
void		 checkbox_scale(void *, int, int);
void		 checkbox_destroy(void *);
void		 checkbox_draw(void *);
void		 checkbox_toggle(struct checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
