/*	$Csoft: checkbox.h,v 1.13 2003/04/25 09:47:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define CHECKBOX_CAPTION_MAX	32

struct checkbox {
	struct widget	wid;
	char		 caption[CHECKBOX_CAPTION_MAX];
	SDL_Surface	*label_s;
	struct {
		int	 state;
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct checkbox	*checkbox_new(struct region *, const char *,
				              ...);
extern DECLSPEC void		 checkbox_init(struct checkbox *, char *);
extern DECLSPEC void		 checkbox_destroy(void *);
extern DECLSPEC void		 checkbox_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
