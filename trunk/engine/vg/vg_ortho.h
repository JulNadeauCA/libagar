/*	$Csoft: vg_ortho.h,v 1.1 2004/04/27 11:43:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_ORTHO_H_
#define _AGAR_VG_ORTHO_H_

#include <engine/widget/toolbar.h>
#include "begin_code.h"

enum vg_ortho_mode {
	VG_NO_ORTHO,		/* No orthogonal restriction */
	VG_HORIZ_ORTHO,		/* Horizontal restriction */
	VG_VERT_ORTHO		/* Vertical restriction */
};

__BEGIN_DECLS
void		VG_RestrictOrtho(struct vg *, double *, double *);
__inline__ void	VG_OrthoRestrictMode(struct vg *, enum vg_ortho_mode);
AG_Toolbar     *VG_OrthoRestrictToolbar(void *, struct vg *,
		                        enum ag_toolbar_type);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ORTHO_H_ */
