/*	$Csoft: scrollbar.h,v 1.7 2002/12/26 07:04:36 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

enum scrollbar_orientation {
	SCROLLBAR_HORIZ,
	SCROLLBAR_VERT
};

struct scrollbar {
	struct widget wid;

	/* Read-only once attached. */
	enum scrollbar_orientation orientation;
	int			   button_size;		/* Scroll button size */

	/* Shares window lock. */
	int	 curbutton;		/* Button held */
	int	 bar_size;		/* Scroll bar size */

	/* Default bindings */
	struct {
		int	 value;			/* Current value */
		int	 min, max;		/* Range */
		pthread_mutex_t	    lock;	/* Lock on default bindings */
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
