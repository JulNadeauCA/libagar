/*	$Csoft: checkbox.h,v 1.9 2002/12/21 10:26:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

struct checkbox {
	struct widget	wid;
	char		*caption;	/* Label string */
	SDL_Surface	*label_s;	/* Label surface */
	int	 	 cbox_w;	/* Checkbox width */
	struct {
		int		 state;	/* Default binding */
	} def;
};

struct checkbox	*checkbox_new(struct region *, int, const char *, ...);
void		 checkbox_init(struct checkbox *, int, char *);
void		 checkbox_destroy(void *);
void		 checkbox_draw(void *);

#endif /* _AGAR_WIDGET_CHECKBOX_H_ */
