/*	$Csoft: vg.h,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_LINE_H_
#define _AGAR_VG_LINE_H_
#include "begin_code.h"

__BEGIN_DECLS
void		vg_draw_lines(struct vg *, struct vg_element *);
void		vg_draw_line_strip(struct vg *, struct vg_element *);
void		vg_draw_line_loop(struct vg *, struct vg_element *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_LINE_H_ */
