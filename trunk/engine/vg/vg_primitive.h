/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_VG_PRIMITIVE_H_
#define _AGAR_VG_PRIMITIVE_H_

#include <engine/view.h>

#include "begin_code.h"

__BEGIN_DECLS
void		vg_putpixel(struct vg *, int, int, Uint32);
void		vg_circle_primitive(struct vg *, int, int, int, Uint32);
void		vg_line_primitive(struct vg *, int, int, int, int, Uint32);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_PRIMITIVE_H_ */
