/*	$Csoft: scrollbar.h,v 1.4 2002/11/04 08:34:13 vedge Exp $	*/
/*	Public domain	*/

struct scrollbar {
	struct widget wid;

	int	 flags;
#define SCROLLBAR_HORIZONTAL	0x01	/* Horizontal scroll bar */
#define SCROLLBAR_VERTICAL	0x02	/* Vertical scroll bar */

	int	 item_size;		/* Item width/height in pixels */
	int	 min_size;
	int	 curbutton;		/* Button held */

	struct {
		int	soft_start;	/* Soft scroll offset */
		int	start;		/* Display offset */
		int	max;		/* Range */
		pthread_mutex_t max_lock;
	} range;
};

struct scrollbar	*scrollbar_new(struct region *, int, int, int, int);
void			 scrollbar_init(struct scrollbar *, int, int, int, int);
void		 	 scrollbar_destroy(void *);
void			 scrollbar_draw(void *);

void			 scrollbar_set_range(struct scrollbar *, int);
void			 scrollbar_set_position(struct scrollbar *, int);

