/*	$Csoft: box.h,v 1.3 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define BOX_WFILL	0x02	/* Expand to fill available width */
#define BOX_HFILL	0x04	/* Expand to fill available height */
#define BOX_FRAME	0x08	/* Display a frame */

enum box_type {
	BOX_HORIZ,
	BOX_VERT
};

struct box {
	struct widget wid;
	enum box_type type;

	pthread_mutex_t	lock;
	int homogenous;			/* Divide space evenly */
	int padding;			/* Padding around widgets */
	int spacing;			/* Spacing between widgets */
	int depth;			/* Depth of frame (for BOX_FRAME) */
};

__BEGIN_DECLS
struct box	*box_new(void *, enum box_type, int);

void	 box_init(struct box *, enum box_type, int);
void	 box_destroy(void *);
void	 box_draw(void *);
void	 box_scale(void *, int, int);

void	 box_set_homogenous(struct box *, int);
void	 box_set_padding(struct box *, int);
void	 box_set_spacing(struct box *, int);
void	 box_set_color(struct box *, Uint8, Uint8, Uint8);
void	 box_set_depth(struct box *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
