/*	$Csoft: button.h,v 1.21 2003/02/02 21:16:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <engine/widget/widget.h>

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

struct button	*button_new(struct region *, char *, SDL_Surface *, int, int,
		     int);
void		 button_init(struct button *, char *, SDL_Surface *, int, int,
		     int);
void		 button_destroy(void *);
void		 button_draw(void *);
void		 button_enable(struct button *);
void		 button_disable(struct button *);

#endif /* _AGAR_WIDGET_BUTTON_H_ */
