/*	$Csoft: radio.h,v 1.2 2002/07/27 07:02:55 vedge Exp $	*/
/*	Public domain	*/

struct radio {
	struct	widget wid;

	int	flags;

	const	char **items;
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

struct radio	*radio_new(struct region *, const char *[], int, int);
void		 radio_init(struct radio *, const char *[], int, int);
void	 	 radio_draw(void *);

