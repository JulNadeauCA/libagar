/*	$Csoft: vg_origin.h,v 1.2 2004/04/19 02:15:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ORIGIN_H_
#define _AGAR_VG_ORIGIN_H_
#include "begin_code.h"

#define VG_NORIGINS	3

__BEGIN_DECLS
void		vg_origin2(struct vg *, int, double, double);
void		vg_origin3(struct vg *, int, double, double, double);
void		vg_origin_color(struct vg *, int, int, int, int);
void		vg_origin_radius(struct vg *, int, float);
__inline__ void vg_draw_origin(struct vg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ORIGIN_H_ */
