/*	$Csoft: vg_math.h,v 1.2 2004/04/19 02:14:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#include <compat/math.h>

__BEGIN_DECLS
__inline__ double vg_rad2deg(double);
__inline__ double vg_deg2rad(double);
__inline__ void vg_car2pol(double, double, double *, double *);
__inline__ void vg_pol2car(double, double, double *, double *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
