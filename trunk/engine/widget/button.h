/*	$Csoft: button.h,v 1.30 2004/09/12 05:51:44 vedge Exp $	*/
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
	enum button_justify justify;	/* Label justification */
	int flags;
#define BUTTON_INSENSITIVE	0x01	/* Not responsive */
#define BUTTON_STICKY		0x02	/* Toggle state */
#define BUTTON_MOUSEOVER	0x04	/* Mouse overlaps */
#define BUTTON_REPEAT		0x08	/* Send multiple button-pushed events */
	int padding;			/* Padding in pixels */
	struct timeout delay_to;	/* Delay for triggering repeat mode */
	struct timeout repeat_to;	/* Timeout for repeat mode */
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
void	 button_set_repeat(struct button *, int);
void	 button_printf(struct button *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BUTTON_H_ */
