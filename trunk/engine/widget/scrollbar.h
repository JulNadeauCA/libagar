/*	$Csoft: scrollbar.h,v 1.12 2003/04/25 09:47:10 vedge Exp $	*/
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
	struct widget	wid;
	int		value;		/* Default value binding */
	int		min, max;	/* Default range binding */

	enum scrollbar_type	type;
	int			button_size;	/* Scroll button size */
	int			curbutton;	/* Button held */
	int			bar_size;	/* Scroll bar size */
};

__BEGIN_DECLS
extern DECLSPEC struct scrollbar *scrollbar_new(void *, enum scrollbar_type);

extern DECLSPEC void	scrollbar_init(struct scrollbar *, enum scrollbar_type);
extern DECLSPEC void	scrollbar_scale(void *, int, int);
extern DECLSPEC void	scrollbar_destroy(void *);
extern DECLSPEC void	scrollbar_draw(void *);

extern __inline__ void	scrollbar_set_bar_size(struct scrollbar *, int);
extern __inline__ void	scrollbar_get_bar_size(struct scrollbar *, int *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
