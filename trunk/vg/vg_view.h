/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_VIEW_H_
#define _AGAR_VG_VIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/menu.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_tool.h>

#include "begin_code.h"

typedef struct vg_view {
	struct ag_widget wid;
	Uint flags;
#define VG_VIEW_HFILL	0x01
#define VG_VIEW_VFILL	0x02
#define VG_VIEW_FOCUS	0x10
#define VG_VIEW_EXPAND	(VG_VIEW_HFILL|VG_VIEW_VFILL)
	VG *vg;					/* Vector drawing */
	int x, y;				/* Display offset */
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
	TAILQ_HEAD(, vg_tool) tools;		/* Map edition tools */
	char status[128];			/* Status text buffer */
} VG_View;

#define VGVIEW(p) ((VG_View *)(p))

__BEGIN_DECLS
VG_View	  *VG_ViewNew(void *, VG *, Uint);
void	   VG_ViewInit(VG_View *, VG *, Uint);
void	   VG_ViewDestroy(void *);
void	   VG_ViewDraw(void *);
void	   VG_ViewScale(void *, int, int);
void	   VG_ViewReshape(VG_View *);
void	   VG_ViewDrawFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewScaleFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewKeydownFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewKeyupFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewButtondownFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewButtonupFn(VG_View *, AG_EventFn, const char *, ...);
void	   VG_ViewMotionFn(VG_View *, AG_EventFn, const char *, ...);

void	   VG_ViewSelectTool(VG_View *, VG_Tool *, void *);
VG_Tool	  *VG_ViewFindTool(VG_View *, const char *);
VG_Tool	  *VG_ViewRegTool(VG_View *, const VG_ToolOps *, void *);
void	   VG_ViewSetDefaultTool(VG_View *, VG_Tool *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_VIEW_H_ */
