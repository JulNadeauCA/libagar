/*	$Csoft: vg_snap.h,v 1.4 2004/10/06 04:56:08 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_SNAP_H_
#define _AGAR_VG_SNAP_H_

#include <engine/widget/toolbar.h>
#include "begin_code.h"

enum vg_snap_mode {
	VG_FREE_POSITIONING,	/* No positional restriction */
	VG_NEAREST_INTEGER,	/* Round to nearest integer */
	VG_GRID,		/* Snap to grid */
	VG_ENDPOINT,		/* Snap to endpoints */
	VG_ENDPOINT_DISTANCE,	/* Snap to given distance from endpoint */
	VG_CLOSEST_POINT,	/* Snap to closest point on entity */
	VG_CENTER_POINT,	/* Snap to center points */
	VG_MIDDLE_POINT,	/* Snap to middle points */
	VG_INTERSECTIONS_AUTO,	/* Snap to intersections automatically */
	VG_INTERSECTIONS_MANUAL	/* Snap to intersections manually */
};

__BEGIN_DECLS
void		 VG_SnapPoint(struct vg *, double *, double *);
__inline__ void	 VG_SetSnapMode(struct vg *, enum vg_snap_mode);
__inline__ void	 VG_DrawGrid(struct vg *);
AG_Toolbar	*VG_SnapToolbar(void *, struct vg *, enum ag_toolbar_type);

#ifdef EDITION
struct ag_menu;
struct ag_menu_item;

void VG_SnapMenu(struct ag_menu *, struct ag_menu_item *, struct vg *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_SNAP_H_ */
