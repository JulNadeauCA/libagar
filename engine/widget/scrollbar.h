/*	$Csoft: scrollbar.h,v 1.8 2002/12/30 06:30:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#include <engine/widget/widget.h>

enum scrollbar_orientation {
	SCROLLBAR_HORIZ,
	SCROLLBAR_VERT
};

struct scrollbar {
	struct widget	wid;

	/* Read-only once attached. */
	enum scrollbar_orientation orientation;
	int			   button_size;		/* Scroll button size */

	/* Shares window lock. */
	int	 curbutton;		/* Button held */
	int	 bar_size;		/* Scroll bar size */

	/* Default binding */
	struct {
		int		    value;	/* Current value */
		int		    min, max;	/* Range */
		pthread_mutex_t	    lock;
		pthread_mutexattr_t lockattr;
	} def;
};

struct scrollbar	*scrollbar_new(struct region *, int, int,
			     enum scrollbar_orientation);
void			 scrollbar_init(struct scrollbar *, int, int,
			     enum scrollbar_orientation);
void		 	 scrollbar_destroy(void *);
void			 scrollbar_draw(void *);

#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
