/*	$Csoft: vg_snap.h,v 1.3 2004/04/26 07:03:46 vedge Exp $	*/
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
void		vg_ortho_restrict(struct vg *, double *, double *);
__inline__ void	vg_ortho_mode(struct vg *, enum vg_ortho_mode);
struct toolbar *vg_ortho_toolbar(void *, struct vg *, enum toolbar_type);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_ORTHO_H_ */
