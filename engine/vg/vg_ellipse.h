/*	$Csoft: vg_circle.h,v 1.1 2004/03/30 16:03:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ELLIPSE_H_
#define _AGAR_VG_ELLIPSE_H_
#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	vg_ellipse_diameter2(struct vg *, double, double);
__inline__ void	vg_ellipse_angle2(struct vg *, double, double);
void		vg_draw_ellipse(struct vg *, struct vg_element *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ELLIPSE_H_ */
