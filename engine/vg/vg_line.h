/*	$Csoft: vg_line.h,v 1.2 2004/04/22 01:45:46 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_LINE_H_
#define _AGAR_VG_LINE_H_
#include "begin_code.h"

__BEGIN_DECLS
void vg_draw_line_segments(struct vg *, struct vg_element *);
void vg_draw_line_strip(struct vg *, struct vg_element *);
void vg_draw_line_loop(struct vg *, struct vg_element *);
float vg_line_intsect(struct vg *, struct vg_element *, double, double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_LINE_H_ */
