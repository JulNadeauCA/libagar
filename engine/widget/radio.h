/*	$Csoft: radio.h,v 1.3 2002/07/29 05:29:29 vedge Exp $	*/
/*	Public domain	*/

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

