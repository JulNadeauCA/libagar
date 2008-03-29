/*	Public domain	*/

#ifndef _AGAR_VG_VIEW_H_
#define _AGAR_VG_VIEW_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/menu.h>
#include <gui/text_cache.h>
#include <vg/vg.h>
#include <vg/vg_tool.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/menu.h>
#include <agar/gui/text_cache.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_tool.h>
#endif

#include "begin_code.h"

typedef struct vg_view {
	struct ag_widget wid;
	Uint flags;
#define VG_VIEW_HFILL	0x01
#define VG_VIEW_VFILL	0x02
#define VG_VIEW_EXPAND	(VG_VIEW_HFILL|VG_VIEW_VFILL)
	VG *vg;					/* Vector graphics object */
	int x, y;				/* Display offset */
	float scale;				/* Display scaling factor */
	float wPixel;				/* Relative pixel size */

	enum vg_snap_mode  snap_mode;		/* Snapping constraint */
	enum vg_ortho_mode ortho_mode;		/* Orthogonal constraint */
	float gridIval;				/* Grid interval */

	AG_Event *draw_ev;			/* Draw callback */
	AG_Event *scale_ev;			/* Scaling/movement event */
	AG_Event *keydown_ev, *keyup_ev;	/* Keyboard events */
	AG_Event *btndown_ev, *btnup_ev;	/* Mouse button events */
	AG_Event *motion_ev;			/* Mouse motion event */
	struct {
		float x, y;			/* Saved coords */
		int panning;			/* Panning mode */
	} mouse;
	VG_Tool *curtool;			/* Selected tool */
	VG_Tool *deftool;			/* Default tool if any */
	AG_TAILQ_HEAD(,vg_tool) tools;		/* Map edition tools */
	char status[128];			/* Status text buffer */
	AG_TextCache *tCache;			/* Text cache for VG_Text */
} VG_View;

#define VGVIEW(p) ((VG_View *)(p))

#define VG_VIEW_X(vv,px) ((px)/(vv)->scale - (vv)->x)
#define VG_VIEW_Y(vv,py) ((py)/(vv)->scale - (vv)->y)

__BEGIN_DECLS
extern AG_WidgetClass vgViewClass;

VG_View	*VG_ViewNew(void *, VG *, Uint);

void     VG_ViewSetScale(struct vg_view *, float);
void     VG_ViewSetSnapMode(struct vg_view *, enum vg_snap_mode);
void     VG_ViewSetOrthoMode(struct vg_view *, enum vg_ortho_mode);
void     VG_ViewSetGridInterval(struct vg_view *, float);

void     VG_ViewDrawFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewScaleFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewKeydownFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewKeyupFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewButtondownFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewButtonupFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewMotionFn(VG_View *, AG_EventFn, const char *, ...);

void     VG_ViewSelectTool(VG_View *, VG_Tool *, void *);
VG_Tool *VG_ViewFindTool(VG_View *, const char *);
VG_Tool *VG_ViewFindToolByOps(VG_View *, const VG_ToolOps *);
VG_Tool	*VG_ViewRegTool(VG_View *, const VG_ToolOps *, void *);
void     VG_ViewSetDefaultTool(VG_View *, VG_Tool *);

static __inline__ void
VG_ApplyConstraints(VG_View *vv, float *x, float *y)
{
	if (vv->snap_mode != VG_FREE_POSITIONING)
		VG_SnapPoint(vv, x, y);
	if (vv->ortho_mode != VG_NO_ORTHO)
		VG_RestrictOrtho(vv, x, y);
}

/* Translate View coordinates to VG coordinates. */
static __inline__ void
VG_GetVGCoords(VG_View *vv, int x, int y, float *vx, float *vy)
{
	*vx = ((float)x - vv->x)/vv->scale;
	*vy = ((float)y - vv->y)/vv->scale;
}

/* Translate VG coordinates to integer view coordinates. */
static __inline__ void
VG_GetViewCoords(VG_View *vv, float vx, float vy, int *x, int *y)
{
	*x = (int)(vx*vv->scale) + vv->x;
	*y = (int)(vy*vv->scale) + vv->y;
}

/*
 * Translate element vertices to floating-point view coordinates after
 * applying the element transformations.
 */
static __inline__ void
VG_GetViewCoordsVtxFlt(VG_View *vv, VG_Element *vge, int vi, float *x, float *y)
{
	VG_Vtx c;
	int i;

	c.x = vge->vtx[vi].x;
	c.y = vge->vtx[vi].y;
	for (i = 0; i < vge->ntrans; i++) {
		VG_MultMatrixByVector(&c, &c, &vge->trans[i]);
	}
	*x = c.x*vv->scale + vv->x;
	*y = c.y*vv->scale + vv->y;
}

/*
 * Translate element vertices to integer view coordinates after applying the
 * element transformations.
 */
static __inline__ void
VG_GetViewCoordsVtx(VG_View *vv, VG_Element *vge, int vi, int *x, int *y)
{
	float vx, vy;

	VG_GetViewCoordsVtxFlt(vv, vge, vi, &vx, &vy);
	*x = (int)vx;
	*y = (int)vy;
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_VIEW_H_ */
