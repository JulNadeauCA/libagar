/*	$Csoft: vg_point.h,v 1.4 2004/04/30 12:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MASK_H_
#define _AGAR_VG_MASK_H_
#include "begin_code.h"

__BEGIN_DECLS
void	vg_mask_init(struct vg *, struct vg_element *);
void	vg_draw_mask(struct vg *, struct vg_element *);
void	vg_mask_scale(struct vg *, float);
void	vg_mask_visible(struct vg *, int);
void	vg_mask_pointer(struct vg *, void *);
void	vg_mask_mousebutton(struct vg *, void (*)(void *, Uint8), void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MASK_H_ */
