/*	Public domain	*/

#ifndef _AGAR_VG_VIEW_H_
#define _AGAR_VG_VIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/text_cache.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_tool.h>

#define VG_GRIDS_MAX 4

#include <agar/vg/begin.h>

enum vg_grid_type {
	VG_GRID_POINTS,
	VG_GRID_LINES
};

typedef struct vg_grid {
	enum vg_grid_type type;
	Uint flags;
#define VG_GRID_HIDE      0x01	/* Hide grid */
#define VG_GRID_UNDERSIZE 0x02	/* Grid too small to display */

	int ival;		/* Nominal pixel interval */
	int ivalView;		/* Effective interval (dependent on wPixel) */
	VG_Color color;		/* Display color */
} VG_Grid;

typedef struct vg_view {
	struct ag_widget wid;

	Uint flags;
#define VG_VIEW_HFILL		0x01
#define VG_VIEW_VFILL		0x02
#define VG_VIEW_GRID		0x04		/* Display grid */
#define VG_VIEW_EXTENTS		0x08		/* Display extents (DEBUG) */
#define VG_VIEW_DISABLE_BG	0x10		/* Enable VG background */
#define VG_VIEW_CONSTRUCTION	0x20		/* Construction geometry */
#define VG_VIEW_EXPAND	(VG_VIEW_HFILL|VG_VIEW_VFILL)

	VG *vg;					/* Vector graphics object */

	float x, y;				/* Display offset */
	int scaleIdx;				/* Scaling factor index */
	float scale;				/* Display scaling factor */
	float scaleMin;				/* Minimum scaling factor */
	float scaleMax;				/* Maximum scaling factor */
	float wPixel;				/* Relative pixel size */

	enum vg_snap_mode  snap_mode;		/* Snapping constraint */
	enum vg_ortho_mode ortho_mode;		/* Orthogonal constraint */
	VG_Grid grid[VG_GRIDS_MAX];		/* Grid settings */
	Uint nGrids;

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
	AG_TAILQ_HEAD_(vg_tool) tools;		/* Map edition tools */

	char status[128];			/* Status text buffer */
	AG_TextCache *tCache;			/* Text cache for VG_Text */
	AG_Widget **editAreas;			/* User-specified container */
	Uint nEditAreas;
	int pointSelRadius;			/* Point selection threshold */
	AG_Rect r;				/* View area */
} VG_View;

#define VGVIEW(p) ((VG_View *)(p))

#define VG_SKIP_CONSTRAINTS(vv) (AG_GetModState(vv) & AG_KEYMOD_SHIFT)
#define VG_SELECT_MULTI(vv)     (AG_GetModState(vv) & AG_KEYMOD_CTRL)

__BEGIN_DECLS
extern AG_WidgetClass vgViewClass;

VG_View	*VG_ViewNew(void *, VG *, Uint);
void     VG_ViewSetVG(struct vg_view *, VG *);
void     VG_ViewSetScale(struct vg_view *, float);
void     VG_ViewSetScalePreset(struct vg_view *, int);
void     VG_ViewSetScaleMin(struct vg_view *, float);
void     VG_ViewSetScaleMax(struct vg_view *, float);
void     VG_ViewSetSnapMode(struct vg_view *, enum vg_snap_mode);
void     VG_ViewSetOrthoMode(struct vg_view *, enum vg_ortho_mode);
void     VG_ViewSetGrid(struct vg_view *, int, enum vg_grid_type, int,
                        VG_Color);

void     VG_ViewDrawFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewScaleFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewKeydownFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewKeyupFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewButtondownFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewButtonupFn(VG_View *, AG_EventFn, const char *, ...);
void     VG_ViewMotionFn(VG_View *, AG_EventFn, const char *, ...);

void     VG_ViewSelectTool(VG_View *, void *, void *);
void     VG_ViewSelectToolEv(AG_Event *);
VG_Tool *VG_ViewFindTool(VG_View *, const char *);
VG_Tool *VG_ViewFindToolByOps(VG_View *, const VG_ToolOps *);
VG_Tool	*VG_ViewRegTool(VG_View *, const VG_ToolOps *, void *);
void     VG_ViewSetDefaultTool(VG_View *, VG_Tool *);
void     VG_StatusS(VG_View *, const char *);
void     VG_Status(VG_View *, const char *, ...)
	    FORMAT_ATTRIBUTE(printf, 2, 3);
Uint     VG_AddEditArea(VG_View *, void *);
void     VG_ClearEditAreas(VG_View *);
void     VG_EditNode(VG_View *, Uint, VG_Node *);
void     VG_DrawSurface(VG_View *, int, int, float, int);

/*
 * Apply snapping constraints to given coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_ApplyConstraints(VG_View *vv, VG_Vector *pos)
{
	VG_Grid *grid;
	float r, ival2;
	
	switch (vv->snap_mode) {
	case VG_GRID:
		grid = &vv->grid[0];
		ival2 = (float)(grid->ival>>1);

		r = VG_Mod(pos->x, grid->ival);
		pos->x -= r;
		if (r > ival2) { pos->x += grid->ival; }
		else if (r < -ival2) { pos->x -= grid->ival; }
	
		r = VG_Mod(pos->y, grid->ival);
		pos->y -= r;
		if (r > ival2) { pos->y += grid->ival; }
		else if (r < -ival2) { pos->y -= grid->ival; }
		break;
	default:
		break;
	}
}

/*
 * Translate integer View coordinates to VG coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetVGCoords(VG_View *vv, int x, int y, VG_Vector *v)
{
	v->x = ((float)x - vv->x)/vv->scale;
	v->y = ((float)y - vv->y)/vv->scale;
}

/*
 * Translate floating-point View coordinates to VG coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetVGCoordsFlt(VG_View *vv, VG_Vector pos, VG_Vector *v)
{
	v->x = (pos.x - vv->x)/vv->scale;
	v->y = (pos.y - vv->y)/vv->scale;
}

/*
 * Translate VG coordinates to integer view coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetViewCoords(VG_View *vv, VG_Vector v, int *x, int *y)
{
	*x = (int)(v.x*vv->scale + vv->x);
	*y = (int)(v.y*vv->scale + vv->y);
}

/*
 * Translate VG coordinates to floating-point view coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetViewCoordsFlt(VG_View *vv, VG_Vector v, float *x, float *y)
{
	*x = v.x*vv->scale + vv->x;
	*y = v.y*vv->scale + vv->y;
}

/* Return the Point nearest to vPos. */
static __inline__ void *
VG_NearestPoint(VG_View *vv, VG_Vector vPos, void *ignore)
{
	float prox, proxNearest = AG_FLT_MAX;
	VG_Node *vn, *vnNearest = NULL;
	VG_Vector v;

	AG_TAILQ_FOREACH(vn, &vv->vg->nodes, list) {
		if (vn->ops->pointProximity == NULL ||
		    vn == ignore ||
		    !VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < vv->grid[0].ival) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	return (vnNearest);
}

/* Return the entity nearest to vPos. */
static __inline__ void *
VG_Nearest(VG_View *vv, VG_Vector vPos)
{
	VG *vg = vv->vg;
	float prox, proxNearest;
	VG_Node *vn, *vnNearest;
	VG_Vector v;

	/* Prioritize points at a fixed distance. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	AG_TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (!VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox <= vv->pointSelRadius) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	if (vnNearest != NULL)
		return (vnNearest);

	/* Fallback to a general query. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	AG_TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (vn->ops->pointProximity == NULL) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < proxNearest) {
			proxNearest = prox;
			vnNearest = vn;
		}
	}
	return (vnNearest);
}

/* Highlight and return the Point nearest to vPos. */
static __inline__ void *
VG_HighlightNearestPoint(VG_View *vv, VG_Vector vPos, void *ignore)
{
	VG *vg = vv->vg;
	float prox, proxNearest = AG_FLT_MAX;
	VG_Node *vn, *vnNearest = NULL;
	VG_Vector v;

	AG_TAILQ_FOREACH(vn, &vg->nodes, list) {
		vn->flags &= ~(VG_NODE_MOUSEOVER);
		if (vn->ops->pointProximity == NULL ||
		    vn == ignore ||
		    !VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < vv->grid[0].ival) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	return (vnNearest);
}
__END_DECLS

#include <agar/vg/close.h>
#endif /* _AGAR_VG_VIEW_H_ */
