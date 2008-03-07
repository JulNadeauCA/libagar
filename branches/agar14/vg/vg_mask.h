/*	$Csoft: vg_mask.h,v 1.3 2005/06/04 04:48:44 vedge Exp $	*/
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
void	VG_MaskScale(struct vg *, float);
void	VG_MaskIsVisible(struct vg *, int);
void	VG_MaskPtr(struct vg *, void *);
void	VG_MaskMouseButton(struct vg *, void (*)(void *, Uint8), void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MASK_H_ */
