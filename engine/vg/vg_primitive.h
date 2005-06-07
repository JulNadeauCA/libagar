/*	$Csoft: vg_primitive.h,v 1.6 2005/06/01 09:06:56 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_PRIMITIVE_H_
#define _AGAR_VG_PRIMITIVE_H_

#include <engine/view.h>

#include "begin_code.h"

__BEGIN_DECLS
__inline__ void	vg_put_pixel(struct vg *, int, int, Uint32);
void		vg_circle_primitive(struct vg *, int, int, int, Uint32);
void		vg_arc_primitive(struct vg *, int, int, int, int, int, int,
		                 Uint32);
void		vg_line_primitive(struct vg *, int, int, int, int, Uint32);
__inline__ void vg_hline_primitive(struct vg *, int, int, int, Uint32);
void		vg_wuline_primitive(struct vg *, double, double, double,
		                    double, int, Uint32);
__inline__ void	vg_rect_primitive(struct vg *, int, int, int, int, Uint32);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_PRIMITIVE_H_ */
