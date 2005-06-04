/*	$Csoft: vg_mask.h,v 1.2 2004/05/12 04:53:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MASK_H_
#define _AGAR_VG_MASK_H_
#include "begin_code.h"

struct vg_mask_args {
	float scale;			/* Scaling factor */
	int visible;			/* Display something */

	void (*mousebutton)(void *p, Uint8 b);
	void *p;
};

__BEGIN_DECLS
void	vg_mask_scale(struct vg *, float);
void	vg_mask_visible(struct vg *, int);
void	vg_mask_pointer(struct vg *, void *);
void	vg_mask_mousebutton(struct vg *, void (*)(void *, Uint8), void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MASK_H_ */
