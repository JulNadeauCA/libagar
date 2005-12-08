/*	$Csoft: vg_ellipse.h,v 1.5 2005/06/04 04:48:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ELLIPSE_H_
#define _AGAR_VG_ELLIPSE_H_
#include "begin_code.h"
		
struct vg_ellipse_args {
	float w, h;		/* Geometry */
	float s, e;		/* Start/end angles (degrees) */
};

__BEGIN_DECLS
__inline__ void	VG_EllipseExtent(struct vg *, float, float);
__inline__ void VG_EllipseArc(struct vg *, float, float);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ELLIPSE_H_ */
