/*	$Csoft: vg_circle.h,v 1.1 2004/03/30 16:03:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_CIRCLE_H_
#define _AGAR_VG_CIRCLE_H_
#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	vg_circle_radius(struct vg *, double);
__inline__ void	vg_circle_diameter(struct vg *, double);
void		vg_draw_circle(struct vg *, struct vg_element *);
void		vg_circle_bbox(struct vg *, struct vg_element *,
		               struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_CIRCLE_H_ */
