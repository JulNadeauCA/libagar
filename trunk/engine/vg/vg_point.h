/*	$Csoft: vg_point.h,v 1.3 2004/04/22 12:36:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_POINTS_H_
#define _AGAR_VG_POINTS_H_
#include "begin_code.h"

__BEGIN_DECLS
void		vg_point_init(struct vg *, struct vg_element *);
void		vg_draw_points(struct vg *, struct vg_element *);
void		vg_points_bbox(struct vg *, struct vg_element *,
		               struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_POINTS_H_ */
