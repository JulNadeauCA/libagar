/*	$Csoft$	*/
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

struct hbox {
	struct box box;
};

struct vbox {
	struct box box;
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

#define VBOX_HOMOGENOUS	BOX_HOMOGENOUS
#define VBOX_WFILL	BOX_WFILL
#define VBOX_HFILL	BOX_HFILL
#define HBOX_HOMOGENOUS	BOX_HOMOGENOUS
#define HBOX_WFILL	BOX_WFILL
#define HBOX_HFILL	BOX_HFILL

#define hbox_new(p, fl)	(struct hbox *)box_new((p), BOX_HORIZ, (fl))
#define vbox_new(p, fl)	(struct vbox *)box_new((p), BOX_VERT, (fl))
#define hbox_init(b, fl) box_init((struct box *)(b), BOX_HORIZ, (fl))
#define vbox_init(b, fl) box_init((struct box *)(b), BOX_VERT, (fl))
#define hbox_set_homogenous(b, fl) box_set_homogenous((struct box *)(b), (fl))
#define vbox_set_homogenous(b, fl) box_set_homogenous((struct box *)(b), (fl))
#define hbox_set_padding(b, pad) box_set_padding((struct box *)(b), (pad))
#define vbox_set_padding(b, pad) box_set_padding((struct box *)(b), (pad))
#define hbox_set_spacing(b, sp) box_set_spacing((struct box *)(b), (sp))
#define vbox_set_spacing(b, sp) box_set_spacing((struct box *)(b), (sp))

#include "close_code.h"
#endif /* _AGAR_WIDGET_BOX_H_ */
