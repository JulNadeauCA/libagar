/*	$Csoft: scrollbar.h,v 1.11 2003/04/17 03:59:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

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
	} def;
};

__BEGIN_DECLS
extern DECLSPEC struct scrollbar *scrollbar_new(struct region *, int, int,
				                enum scrollbar_orientation);
extern DECLSPEC void		  scrollbar_init(struct scrollbar *, int, int,
				                 enum scrollbar_orientation);
extern DECLSPEC void		  scrollbar_destroy(void *);
extern DECLSPEC void		  scrollbar_draw(void *);
extern __inline__ void		  scrollbar_set_bar_size(struct scrollbar *,
				                         int);
extern __inline__ void		  scrollbar_get_bar_size(struct scrollbar *,
				                         int *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
