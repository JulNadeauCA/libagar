/*	$Csoft: vg_line.h,v 1.1 2004/03/30 16:03:58 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_LINE_H_
#define _AGAR_VG_LINE_H_
#include "begin_code.h"

__BEGIN_DECLS
void vg_draw_line_segments(struct vg *, struct vg_element *);
void vg_draw_line_strip(struct vg *, struct vg_element *);
void vg_draw_line_loop(struct vg *, struct vg_element *);
void vg_line_bbox(struct vg *, struct vg_element *, struct vg_rect *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_LINE_H_ */
