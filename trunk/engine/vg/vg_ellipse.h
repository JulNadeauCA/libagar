/*	$Csoft: vg_ellipse.h,v 1.3 2004/04/22 12:36:09 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ELLIPSE_H_
#define _AGAR_VG_ELLIPSE_H_
#include "begin_code.h"
		
struct vg_ellipse_args {
	double w, h;		/* Geometry */
	double s, e;		/* Start/end angles (degrees) */
};

__BEGIN_DECLS
void		vg_ellipse_init(struct vg *, struct vg_element *);
__inline__ void	vg_ellipse_diameter2(struct vg *, double, double);
__inline__ void	vg_ellipse_angle2(struct vg *, double, double);
void		vg_draw_ellipse(struct vg *, struct vg_element *);
void		vg_ellipse_bbox(struct vg *, struct vg_element *,
		                struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ELLIPSE_H_ */
