/*	$Csoft: vg_ellipse.h,v 1.5 2005/06/04 04:48:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ARC_H_
#define _AGAR_VG_ARC_H_
#include "begin_code.h"
		
struct vg_arc_args {
	float w, h;		/* Bounding box */
	float s, e;		/* Start/end angles (degrees) */
};

__BEGIN_DECLS
__inline__ void VG_ArcBox(struct vg *, float, float);
__inline__ void VG_ArcRange(struct vg *, float, float);
__inline__ void VG_Arc3Points(struct vg *, VG_Vtx v[3]);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ARC_H_ */
