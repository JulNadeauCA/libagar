/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#include <compat/math.h>

__BEGIN_DECLS
__inline__ double VG_Rad2Deg(double);
__inline__ double VG_Deg2Rad(double);
__inline__ void VG_Car2Pol(struct vg *, double, double, double *, double *);
__inline__ void VG_Pol2Car(struct vg *, double, double, double *, double *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
