/*	$Csoft: button.h,v 1.25 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

enum button_justify {
	BUTTON_LEFT,
	BUTTON_CENTER,
	BUTTON_RIGHT
};

struct button {
	struct widget wid;

	int			 state;		/* Default state binding */
	SDL_Surface		*label;		/* Label */

	int			 sensitive;	/* Service events? */
	int			 sticky;	/* Sticky behavior? */
	int			 padding;	/* Padding in pixels */
	enum button_justify	 justify;	/* Label justification */
};

__BEGIN_DECLS
extern DECLSPEC struct button	*button_new(void *, const char *);

extern DECLSPEC void	 button_init(struct button *, const char *);
extern DECLSPEC void	 button_destroy(void *);
extern DECLSPEC void	 button_draw(void *);
extern DECLSPEC void	 button_scale(void *, int, int);

extern DECLSPEC void	 button_enable(struct button *);
extern DECLSPEC void	 button_disable(struct button *);
extern DECLSPEC void	 button_set_padding(struct button *, int);
extern DECLSPEC void	 button_set_focusable(struct button *, int);
extern DECLSPEC void	 button_set_sticky(struct button *, int);
extern DECLSPEC void	 button_set_justify(struct button *,
			                    enum button_justify);
extern DECLSPEC void	 button_set_label(struct button *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */
