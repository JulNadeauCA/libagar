/*	$Csoft: radio.h,v 1.5 2002/12/21 10:26:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

struct radio {
	struct widget	wid;

	SDL_Surface	**labels;
	const char	**items;
	int		 nitems;
	int		 selitem;	/* Index of selected item */
	int		 max_w;		/* Width of widest label */

	Uint8	 xspacing;
	Uint8	 yspacing;
	Uint8	 radius;

	struct {
		int	value;
	} def;
};

struct radio	*radio_new(struct region *, const char *[]);
void		 radio_init(struct radio *, const char *[]);
void	 	 radio_draw(void *);
void		 radio_destroy(void *);

#endif /* _AGAR_WIDGET_RADIO_H_ */
