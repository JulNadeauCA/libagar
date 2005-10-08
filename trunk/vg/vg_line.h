/*	$Csoft: vg_line.h,v 1.4 2005/06/06 01:01:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_LINE_H_
#define _AGAR_VG_LINE_H_
#include "begin_code.h"

__BEGIN_DECLS
void VG_DrawLineSegments(struct vg *, struct vg_element *);
void VG_DrawLineStrip(struct vg *, struct vg_element *);
void VG_DrawLineLoop(struct vg *, struct vg_element *);
float VG_LineIntersect(struct vg *, struct vg_element *, double, double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_LINE_H_ */
