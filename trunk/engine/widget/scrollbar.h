/*	$Csoft: scrollbar.h,v 1.13 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

enum scrollbar_type {
	SCROLLBAR_HORIZ,
	SCROLLBAR_VERT
};

struct scrollbar {
	struct widget wid;

	int	value;			/* Default value binding */
	int	min, max;		/* Default range binding */

	enum scrollbar_type type;

	int	button_size;		/* Scroll button size */
	int	curbutton;		/* Button held */
	int	bar_size;		/* Scroll bar size */
};

__BEGIN_DECLS
struct scrollbar *scrollbar_new(void *, enum scrollbar_type);

void	scrollbar_init(struct scrollbar *, enum scrollbar_type);
void	scrollbar_scale(void *, int, int);
void	scrollbar_destroy(void *);
void	scrollbar_draw(void *);

__inline__ void	scrollbar_set_bar_size(struct scrollbar *, int);
__inline__ void	scrollbar_get_bar_size(struct scrollbar *, int *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
