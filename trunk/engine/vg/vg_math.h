/*	$Csoft: vg_math.h,v 1.1 2004/03/25 05:36:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#include <compat/math.h>

__BEGIN_DECLS
__inline__ double rad2deg(double);
__inline__ double deg2rad(double);
__inline__ void car2pol(double, double, double *, double *);
__inline__ void pol2car(double, double, double *, double *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
