/*	$Csoft: radio.h,v 1.4 2002/09/07 04:35:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_RADIO_H_
#define _AGAR_WIDGET_RADIO_H_

struct radio {
	struct	widget wid;

	char	**items;
	int	nitems;
	int	selitem;	/* Index of selected item */

	int	xspacing;	/* Horiz spacing */
	int	yspacing;	/* Vert spacing */

	struct {
		int w, h;	
	} radio;

	enum {
		RADIO_LEFT,	/* Left of label */
		RADIO_RIGHT	/* Right of label */
	} justify;
};

struct radio	*radio_new(struct region *, char *[], int);
void		 radio_init(struct radio *, char *[], int);
void	 	 radio_draw(void *);

#endif /* _AGAR_WIDGET_RADIO_H_ */
