/*	$Csoft: vg.h,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_POINTS_H_
#define _AGAR_VG_POINTS_H_
#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	vg_point_radius(struct vg *, double);
__inline__ void	vg_point_diameter(struct vg *, double);
void		vg_draw_points(struct vg *, struct vg_element *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_POINTS_H_ */
