/*	$Csoft: vg_line.h,v 1.2 2004/04/22 01:45:46 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_POLYGON_H_
#define _AGAR_VG_POLYGON_H_
#include "begin_code.h"

struct vg_polygon_args {
	int outline;
};

__BEGIN_DECLS
void vg_polygon_init(struct vg *, struct vg_element *);
void vg_draw_polygon(struct vg *, struct vg_element *);
void vg_polygon_bbox(struct vg *, struct vg_element *, struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_POLYGON_H_ */
