/*	$Csoft: vg_circle.h,v 1.4 2004/05/12 04:53:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_CIRCLE_H_
#define _AGAR_VG_CIRCLE_H_
#include "begin_code.h"
		
struct vg_circle_args {
	double radius;
};

__BEGIN_DECLS
__inline__ void	vg_circle_radius(struct vg *, double);
__inline__ void	vg_circle_diameter(struct vg *, double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_CIRCLE_H_ */
