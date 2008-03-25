/*	Public domain	*/

#ifndef _AGAR_VG_ARC_H_
#define _AGAR_VG_ARC_H_
#include "begin_code.h"
		
struct vg_arc_args {
	float w, h;		/* Bounding box */
	float s, e;		/* Start/end angles (degrees) */
};

__BEGIN_DECLS
void VG_ArcBox(struct vg *, float, float);
void VG_ArcRange(struct vg *, float, float);
void VG_Arc3Points(struct vg *, VG_Vtx v[3]);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ARC_H_ */
