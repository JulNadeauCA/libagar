/*	$Csoft: scrollbar.h,v 1.5 2002/11/20 04:09:50 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

struct scrollbar {
	struct widget wid;

	/* Read-only once attached. */
	int	 flags;
#define SCROLLBAR_HORIZONTAL	0x01	/* Horizontal scroll bar */
#define SCROLLBAR_VERTICAL	0x02	/* Vertical scroll bar */

	int	 curbutton;		/* Button held */
	int	 bar_size;		/* Scroll bar size */
	int	 button_size;		/* Scroll button size */

	int	 value;			/* Current value */
	int	 min, max;		/* Range */
	pthread_mutex_t lock;
};

struct scrollbar	*scrollbar_new(struct region *, int, int, int);
void			 scrollbar_init(struct scrollbar *, int, int, int);
void		 	 scrollbar_destroy(void *);
void			 scrollbar_draw(void *);

void	scrollbar_set_value(struct scrollbar *, int);
void	scrollbar_get_value(struct scrollbar *, int *);
void	scrollbar_set_range(struct scrollbar *, int, int);
void	scrollbar_get_range(struct scrollbar *, int *, int *);

#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
