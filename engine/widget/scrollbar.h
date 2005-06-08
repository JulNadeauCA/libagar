/*	$Csoft: scrollbar.h,v 1.14 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

enum ag_scrollbar_type {
	AG_SCROLLBAR_HORIZ,
	AG_SCROLLBAR_VERT
};

typedef struct ag_scrollbar {
	struct ag_widget wid;
	int value;			/* Default value binding */
	int min, max;			/* Default range binding */
	enum ag_scrollbar_type type;
	int button_size;		/* Scroll button size */
	int curbutton;			/* Button held */
	int bar_size;			/* Scroll bar size */
} AG_Scrollbar;

__BEGIN_DECLS
AG_Scrollbar	*AG_ScrollbarNew(void *, enum ag_scrollbar_type);
void		 AG_ScrollbarInit(AG_Scrollbar *, enum ag_scrollbar_type);
void		 AG_ScrollbarScale(void *, int, int);
void		 AG_ScrollbarDraw(void *);
__inline__ void	 AG_ScrollbarSetBarSize(AG_Scrollbar *, int);
__inline__ void	 AG_ScrollbarGetBarSize(AG_Scrollbar *, int *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */
