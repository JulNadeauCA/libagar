/*	$Csoft: button.h,v 1.22 2003/03/20 01:19:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct button {
	struct widget	 wid;

	int		 flags;
#define BUTTON_STICKY	0x01
#define BUTTON_NOFOCUS	0x02
#define BUTTON_DISABLED	0x04

	char		*caption;	/* String, or NULL */
	SDL_Surface	*label_s;	/* Label (or image) */
	SDL_Surface	*slabel_s;	/* Scaled label surface */
	enum {
		BUTTON_LEFT,
		BUTTON_CENTER,
		BUTTON_RIGHT
	} justify;
	struct {			/* Default binding */
		int	state;
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct button	*button_new(struct region *, char *,
				            SDL_Surface *, int, int, int);
extern DECLSPEC void		 button_init(struct button *, char *,
				             SDL_Surface *, int, int, int);
extern DECLSPEC void		 button_destroy(void *);
extern DECLSPEC void		 button_draw(void *);
extern DECLSPEC void		 button_enable(struct button *);
extern DECLSPEC void		 button_disable(struct button *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */
