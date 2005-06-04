/*	$Csoft: vg_ellipse.h,v 1.4 2004/05/12 04:53:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ELLIPSE_H_
#define _AGAR_VG_ELLIPSE_H_
#include "begin_code.h"
		
struct vg_ellipse_args {
	double w, h;		/* Geometry */
	double s, e;		/* Start/end angles (degrees) */
};

__BEGIN_DECLS
__inline__ void	vg_ellipse_diameter2(struct vg *, double, double);
__inline__ void	vg_ellipse_angle2(struct vg *, double, double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ELLIPSE_H_ */
