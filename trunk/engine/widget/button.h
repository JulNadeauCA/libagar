/*	$Csoft: button.h,v 1.17 2002/12/21 10:26:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BUTTON_H_
#define _AGAR_WIDGET_BUTTON_H_

struct button {
	struct widget	 wid;

	int		 flags;
#define BUTTON_STICKY	0x01

	char		*caption;	/* String, or NULL */
	SDL_Surface	*label_s;	/* Label (or image) */
	SDL_Surface	*slabel_s;	/* Scaled label surface */
	enum {
		BUTTON_LEFT,
		BUTTON_CENTER,
		BUTTON_RIGHT
	} justify;
	struct {			/* Default binding */
		int	value;
	} def;
};

struct button	*button_new(struct region *, char *, SDL_Surface *, int, int,
		     int);
void		 button_init(struct button *, char *, SDL_Surface *, int, int,
		     int);
void		 button_destroy(void *);
void		 button_draw(void *);

#endif /* _AGAR_WIDGET_BUTTON_H_ */
