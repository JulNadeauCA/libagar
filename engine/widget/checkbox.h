/*	$Csoft: checkbox.h,v 1.14 2003/05/26 03:02:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define CHECKBOX_CAPTION_MAX	64

struct checkbox {
	struct widget	 wid;
	int		 state;		/* 1=pressed, 0=released */
	SDL_Surface	*label_s;	/* Text label */
};

__BEGIN_DECLS
extern DECLSPEC struct checkbox	*checkbox_new(void *, const char *, ...);
extern DECLSPEC void		 checkbox_init(struct checkbox *, char *);
extern DECLSPEC void		 checkbox_scale(void *, int, int);
extern DECLSPEC void		 checkbox_destroy(void *);
extern DECLSPEC void		 checkbox_draw(void *);
extern DECLSPEC void		 checkbox_toggle(struct checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
