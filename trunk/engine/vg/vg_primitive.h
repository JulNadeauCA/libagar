/*	$Csoft: vg_primitive.h,v 1.3 2004/04/19 02:09:44 vedge Exp $	*/
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
__inline__ void	vg_rect_primitive(struct vg *, int, int, int, int, Uint32);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_PRIMITIVE_H_ */
