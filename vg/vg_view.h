/*	Public domain	*/

#ifndef _AGAR_VG_VIEW_H_
#define _AGAR_VG_VIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/text_cache.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_tool.h>

#ifndef VG_GRIDS_MAX
#define VG_GRIDS_MAX 4
#endif

#ifndef VG_VIEW_STATUS_MAX
#define VG_VIEW_STATUS_MAX 124
#endif

#include <agar/vg/begin.h>

typedef enum vg_grid_type {
	VG_GRID_POINTS,
	VG_GRID_LINES
} VG_GridType;

typedef struct vg_grid {
	VG_GridType type;	/* Points or lines? */

	Uint flags;
#define VG_GRID_HIDE      0x01	/* Hide grid */
#define VG_GRID_UNDERSIZE 0x02	/* Grid too small to display */

	int ival;		/* Nominal pixel interval */
	int ivalView;		/* Effective interval (dependent on wPixel) */
	VG_Color color;		/* Display color */
} VG_Grid;

typedef struct vg_view {
	struct ag_widget wid;			/* AG_Widget(3) -> VG_View */
	Uint flags;
#define VG_VIEW_HFILL		0x01
#define VG_VIEW_VFILL		0x02
#define VG_VIEW_GRID		0x04		/* Display grid */
#define VG_VIEW_EXTENTS		0x08		/* Display extents (DEBUG) */
#define VG_VIEW_DISABLE_BG	0x10		/* Enable VG background */
#define VG_VIEW_CONSTRUCTION	0x20		/* Construction geometry */
#define VG_VIEW_EXPAND	(VG_VIEW_HFILL|VG_VIEW_VFILL)
	int scaleIdx;				/* Scaling factor index */
	VG *_Nullable vg;			/* Vector graphics object */
	float x, y;				/* Display offset */
	float scale;				/* Display scaling factor */
	float scaleMin;				/* Minimum scaling factor */
	float scaleMax;				/* Maximum scaling factor */
	float wPixel;				/* Relative pixel size */
	enum vg_snap_mode  snap_mode;		/* Snapping constraint */
	VG_Grid grid[VG_GRIDS_MAX];		/* Grid settings */
	Uint   nGrids;
	AG_Event *_Nullable draw_ev;		/* Draw callback */
	AG_Event *_Nullable scale_ev;		/* Scaling/movement event */
	AG_Event *_Nullable keydown_ev;		/* Key press event */
	AG_Event *_Nullable keyup_ev;		/* Key release event */
	AG_Event *_Nullable btndown_ev;		/* Mouse button down events */
	AG_Event *_Nullable btnup_ev;		/* Mouse button up events */
	AG_Event *_Nullable motion_ev;		/* Mouse motion event */

	struct {
		float x, y;			/* Saved coords */
		int panning;			/* Panning mode */
	} mouse;

	char status[VG_VIEW_STATUS_MAX];	/* Status text buffer */

	VG_Tool *_Nullable curtool;		/* Selected tool */
	VG_Tool *_Nullable deftool;		/* Default tool if any */
	AG_TAILQ_HEAD_(vg_tool) tools;		/* Edition tools */

	AG_TextCache *_Nonnull tCache;		/* Text cache for VG_Text */

	AG_Widget *_Nullable *_Nonnull editAreas; /* User-created containers */
	Uint                          nEditAreas;

	int pointSelRadius;			/* Point selection threshold */
	AG_Rect r;				/* View area */
} VG_View;

#define VGVIEW(p)              ((VG_View *)(p))
#define VGCVIEW(p)             ((const VG_View *)(p))
#define VG_VIEW_SELF()          VGVIEW( AG_OBJECT(0,"AG_Widget:VG_View:*") )
#define VG_VIEW_PTR(n)          VGVIEW( AG_OBJECT((n),"AG_Widget:VG_View:*") )
#define VG_VIEW_NAMED(n)        VGVIEW( AG_OBJECT_NAMED((n),"AG_Widget:VG_View:*") )
#define VG_CONST_VIEW_SELF()   VGCVIEW( AG_CONST_OBJECT(0,"AG_Widget:VG_View:*") )
#define VG_CONST_VIEW_PTR(n)   VGCVIEW( AG_CONST_OBJECT((n),"AG_Widget:VG_View:*") )
#define VG_CONST_VIEW_NAMED(n) VGCVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:VG_View:*") )

#define VG_SKIP_CONSTRAINTS(vv) (AG_GetModState(vv) & AG_KEYMOD_SHIFT)
#define VG_SELECT_MULTI(vv)     (AG_GetModState(vv) & AG_KEYMOD_CTRL)

__BEGIN_DECLS
extern AG_WidgetClass vgViewClass;

VG_View	*_Nonnull VG_ViewNew(void *_Nullable, VG *_Nonnull, Uint);

void VG_ViewSetVG(struct vg_view *_Nonnull, VG *_Nullable);
void VG_ViewSetScale(struct vg_view *_Nonnull, float);
void VG_ViewSetScalePreset(struct vg_view *_Nonnull, int);
void VG_ViewSetScaleMin(struct vg_view *_Nonnull, float);
void VG_ViewSetScaleMax(struct vg_view *_Nonnull, float);
void VG_ViewSetSnapMode(struct vg_view *_Nonnull, enum vg_snap_mode);
void VG_ViewSetGrid(struct vg_view *_Nonnull, int, VG_GridType, int, VG_Color);

void VG_ViewDrawFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewScaleFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewKeydownFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewKeyupFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewButtondownFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewButtonupFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void VG_ViewMotionFn(VG_View *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);

void VG_ViewSelectTool(VG_View *_Nonnull, void *_Nullable, void *_Nullable);
void VG_ViewSelectToolEv(AG_Event *_Nonnull);

VG_Tool *_Nullable VG_ViewFindTool(VG_View *_Nonnull, const char *_Nonnull);
VG_Tool *_Nullable VG_ViewFindToolByOps(VG_View *_Nonnull, const VG_ToolOps *_Nonnull);
VG_Tool	*_Nonnull  VG_ViewRegTool(VG_View *_Nonnull, const VG_ToolOps *_Nonnull,
                                  void *_Nullable);

void VG_ViewSetDefaultTool(VG_View *_Nonnull, VG_Tool *_Nullable);
void VG_StatusS(VG_View *_Nonnull, const char *_Nullable);
void VG_Status(VG_View *_Nonnull, const char *_Nullable, ...)
              FORMAT_ATTRIBUTE(printf,2,3);
Uint VG_AddEditArea(VG_View *_Nonnull, void *_Nonnull);
void VG_ClearEditAreas(VG_View *_Nonnull);
void VG_EditNode(VG_View *_Nonnull, Uint, VG_Node *_Nonnull);
void VG_DrawSurface(VG_View *_Nonnull, int,int, float, int);
void VG_ApplyConstraints(VG_View *_Nonnull, VG_Vector *_Nonnull);

void *_Nullable VG_NearestPoint(VG_View *_Nonnull, VG_Vector, void *_Nullable);
void *_Nullable VG_Nearest(VG_View *_Nonnull, VG_Vector);
void *_Nullable VG_HighlightNearestPoint(VG_View *_Nonnull, VG_Vector, void *_Nullable);

/*
 * Translate integer View coordinates to VG coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetVGCoords(VG_View *_Nonnull vv, int x, int y, VG_Vector *_Nonnull v)
{
	v->x = ((float)x - vv->x)/vv->scale;
	v->y = ((float)y - vv->y)/vv->scale;
}

/*
 * Translate floating-point View coordinates to VG coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetVGCoordsFlt(VG_View *_Nonnull vv, VG_Vector pos, VG_Vector *_Nonnull v)
{
	v->x = (pos.x - vv->x)/vv->scale;
	v->y = (pos.y - vv->y)/vv->scale;
}

/*
 * Translate VG coordinates to integer view coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetViewCoords(VG_View *_Nonnull vv, VG_Vector v,
    int *_Nonnull x, int *_Nonnull y)
{
	*x = (int)(v.x*vv->scale + vv->x);
	*y = (int)(v.y*vv->scale + vv->y);
}

/*
 * Translate VG coordinates to floating-point view coordinates.
 * VG_View must be locked.
 */
static __inline__ void
VG_GetViewCoordsFlt(VG_View *_Nonnull vv, VG_Vector v,
    float *_Nonnull x, float *_Nonnull y)
{
	*x = v.x*vv->scale + vv->x;
	*y = v.y*vv->scale + vv->y;
}
__END_DECLS

#include <agar/vg/close.h>
#endif /* _AGAR_VG_VIEW_H_ */
