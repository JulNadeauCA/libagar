/*	$Csoft: box.h,v 1.5 2005/03/09 06:39:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define AG_BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define AG_BOX_WFILL		0x02	/* Expand to fill available width */
#define AG_BOX_HFILL		0x04	/* Expand to fill available height */
#define AG_BOX_FRAME		0x08	/* Display a frame */

enum ag_box_type {
	AG_BOX_HORIZ,
	AG_BOX_VERT
};

typedef struct ag_box {
	struct ag_widget wid;
	enum ag_box_type type;
	pthread_mutex_t	lock;
	int homogenous;			/* Divide space evenly */
	int padding;			/* Padding around widgets */
	int spacing;			/* Spacing between widgets */
	int depth;			/* Depth of frame (for AG_BOX_FRAME) */
} AG_Box;

__BEGIN_DECLS
AG_Box	*AG_BoxNew(void *, enum ag_box_type, int);
void	 AG_BoxInit(AG_Box *, enum ag_box_type, int);
void	 AG_BoxDestroy(void *);
void	 AG_BoxDraw(void *);
void	 AG_BoxScale(void *, int, int);

void	 AG_BoxSetHomogenous(AG_Box *, int);
void	 AG_BoxSetPadding(AG_Box *, int);
void	 AG_BoxSetSpacing(AG_Box *, int);
void	 AG_BoxSetDepth(AG_Box *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
