/*	$Csoft: vg_math.h,v 1.3 2004/04/30 05:21:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#include <compat/math.h>

__BEGIN_DECLS
__inline__ double vg_rad2deg(double);
__inline__ double vg_deg2rad(double);
__inline__ void vg_car2pol(struct vg *, double, double, double *, double *);
__inline__ void vg_pol2car(struct vg *, double, double, double *, double *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
