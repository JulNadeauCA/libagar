/*	$Csoft: vg_point.h,v 1.1 2004/03/30 16:04:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_POINTS_H_
#define _AGAR_VG_POINTS_H_
#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	vg_point_radius(struct vg *, double);
__inline__ void	vg_point_diameter(struct vg *, double);
void		vg_draw_points(struct vg *, struct vg_element *);
void		vg_points_bbox(struct vg *, struct vg_element *,
		               struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_POINTS_H_ */
