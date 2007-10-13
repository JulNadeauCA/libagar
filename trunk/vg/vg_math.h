/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#define VG_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define VG_Norm2(ax,ay) sqrtf(VG_DotProd2((ax),(ay),(ax),(ay)))
#define VG_Distance2(ax,ay,bx,by) VG_Norm2((float)((ax)-(bx)),(float)((ay)-(by)))

__BEGIN_DECLS
__inline__ float VG_Rad2Deg(float);
__inline__ float VG_Deg2Rad(float);
__inline__ void VG_Car2Pol(struct vg *, float, float, float *, float *);
__inline__ void VG_Pol2Car(struct vg *, float, float, float *, float *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
