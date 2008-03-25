/*	Public domain	*/

#ifndef _AGAR_VG_LINE_H_
#define _AGAR_VG_LINE_H_
#include "begin_code.h"

__BEGIN_DECLS
void VG_DrawLineSegments(struct vg_view *, struct vg_element *);
void VG_DrawLineStrip(struct vg_view *, struct vg_element *);
void VG_DrawLineLoop(struct vg_view *, struct vg_element *);
void VG_LineExtent(struct vg *, struct vg_element *, struct vg_rect *);
float VG_LineIntersect(struct vg *, struct vg_element *, float *, float *);
float VG_ClosestLinePoint(struct vg *, float, float, float, float, float *, 
                          float *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_LINE_H_ */
