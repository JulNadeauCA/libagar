/*	$Csoft: button.h,v 1.20 2003/01/25 06:21:54 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

#include <engine/widget/widget.h>

struct button {
	struct widget	 wid;

	int		 flags;
#define BUTTON_STICKY	0x01
#define BUTTON_NOFOCUS	0x02

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

#endif /* _AGAR_WIDGET_BUTTON_H_ */
