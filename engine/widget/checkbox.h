/*	$Csoft: checkbox.h,v 1.8 2002/06/20 16:36:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

struct checkbox {
	struct	 widget wid;

	int	 flags;
#define CHECKBOX_PRESSED	0x01

	char		*caption;
	SDL_Surface	*label_s;

	int	 xspacing;	/* Horiz spacing */
	int	 cbox_w;	/* Checkbox width */

	enum {
		CHECKBOX_LEFT,	/* Left of label */
		CHECKBOX_RIGHT	/* Right of label */
	} justify;
};

struct checkbox	*checkbox_new(struct region *, char *, int, int);
void		 checkbox_init(struct checkbox *, char *, int, int);
void		 checkbox_destroy(void *);

void	 checkbox_draw(void *);

#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
