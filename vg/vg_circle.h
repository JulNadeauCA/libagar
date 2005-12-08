/*	$Csoft: vg_circle.h,v 1.5 2005/06/04 04:48:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_CIRCLE_H_
#define _AGAR_VG_CIRCLE_H_
#include "begin_code.h"
		
struct vg_circle_args {
	float radius;
};

__BEGIN_DECLS
__inline__ void	VG_CircleRadius(struct vg *, float);
__inline__ void	VG_CircleDiameter(struct vg *, float);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_CIRCLE_H_ */
