/*	$Csoft: radio.h,v 1.1 2002/07/07 00:25:13 vedge Exp $	*/
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

enum {
	RADIO_INSIDE = 0,
	RADIO_OUTSIDE = 1,
	RADIO_TEXT = 2
};

struct radio	*radio_new(struct region *, const char *[], int, int);
void		 radio_init(struct radio *, const char *[], int, int);
void	 	 radio_draw(void *);

