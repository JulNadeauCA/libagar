/*	$Csoft: vg_primitive.h,v 1.7 2005/06/07 02:42:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_PRIMITIVE_H_
#define _AGAR_VG_PRIMITIVE_H_

#include <engine/view.h>

#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	VG_PutPixel(VG *, int, int, Uint32);
void		VG_CirclePrimitive(VG *, int, int, int, Uint32);
void		VG_ArcPrimitive(VG *, int, int, int, int, int, int,
		                 Uint32);
void		VG_LinePrimitive(VG *, int, int, int, int, Uint32);
__inline__ void VG_HLinePrimitive(VG *, int, int, int, Uint32);
void		VG_WuLinePrimitive(VG *, double, double, double,
		                    double, int, Uint32);
__inline__ void	VG_RectPrimitive(VG *, int, int, int, int, Uint32);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_PRIMITIVE_H_ */
