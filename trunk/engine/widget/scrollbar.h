/*	$Csoft: scrollbar.h,v 1.12 2002/08/21 23:51:39 vedge Exp $	*/
/*	Public domain	*/

struct scrollbar {
	struct widget wid;

	int	 flags;
#define SCROLLBAR_HORIZONTAL	0x01	/* Horizontal scroll bar */
#define SCROLLBAR_VERTICAL	0x02	/* Vertical scroll bar */

	int	 item_size;		/* Item width/height in pixels */

	struct {
		int	soft_start;	/* Soft scroll offset */
		int	start;		/* Display offset */
		int	max;		/* Range */
	} range;
};

void			 scrollbar_set_range(struct scrollbar *, int);
struct scrollbar	*scrollbar_new(struct region *, int, int, int, int);
void			 scrollbar_init(struct scrollbar *, int, int, int, int);
void		 	 scrollbar_destroy(void *);
void			 scrollbar_draw(void *);

