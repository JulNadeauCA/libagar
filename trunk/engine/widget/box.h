/*	$Csoft: box.h,v 1.1 2003/06/06 02:57:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BOX_H_
#define _AGAR_WIDGET_BOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

#define BOX_HOMOGENOUS	0x01	/* Divide space evenly */
#define BOX_WFILL	0x02	/* Expand to fill available width */
#define BOX_HFILL	0x04	/* Expand to fill available height */

enum box_type {
	BOX_HORIZ,
	BOX_VERT
};

struct box {
	struct widget wid;

	enum box_type	 type;

	pthread_mutex_t	 lock;
	int		 homogenous;	/* Divide space evenly */
	int		 padding;	/* Padding around widgets */
	int		 spacing;	/* Spacing between widgets */
};

__BEGIN_DECLS
extern DECLSPEC struct box	*box_new(void *, enum box_type, int);
extern DECLSPEC void		 box_init(struct box *, enum box_type, int);
extern DECLSPEC void		 box_destroy(void *);
extern DECLSPEC void		 box_draw(void *);
extern DECLSPEC void		 box_scale(void *, int, int);

extern DECLSPEC void		 box_set_homogenous(struct box *, int);
extern DECLSPEC void		 box_set_padding(struct box *, int);
extern DECLSPEC void		 box_set_spacing(struct box *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
