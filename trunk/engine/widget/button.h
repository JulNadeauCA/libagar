/*	$Csoft: button.h,v 1.28 2003/11/10 22:40:47 vedge Exp $	*/
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
	int state;			/* Default state binding */

	SDL_Surface *label;		/* Label surface */
	enum button_justify justify;	/* Label justification */

	int sensitive;			/* Service events? */
	int sticky;			/* Sticky behavior? */
	int padding;			/* Padding in pixels */
	int moverlap;			/* Cursor overlapping */
};

__BEGIN_DECLS
struct button	*button_new(void *, const char *);

void	 button_init(struct button *, const char *);
void	 button_destroy(void *);
void	 button_draw(void *);
void	 button_scale(void *, int, int);

void	 button_enable(struct button *);
void	 button_disable(struct button *);
void	 button_set_padding(struct button *, int);
void	 button_set_focusable(struct button *, int);
void	 button_set_sticky(struct button *, int);
void	 button_set_justify(struct button *, enum button_justify);
void	 button_set_label(struct button *, SDL_Surface *);
void	 button_printf(struct button *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */
