/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SK_VIEW_H_
#define _AGAR_SK_VIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/menu.h>
#include <agar/sg/sk_tool.h>

#include "begin_code.h"

struct sk;

typedef struct sk_view {
	struct ag_widget wid;
	Uint flags;
#define SK_VIEW_HFILL	0x01
#define SK_VIEW_VFILL	0x02
#define SK_VIEW_FOCUS	0x10
#define SK_VIEW_EXPAND	(SK_VIEW_HFILL|SK_VIEW_VFILL)
	struct sk *sk;				/* Sketch object */
	SG_Matrix mView;			/* Viewing matrix */
	SG_Matrix mProj;			/* Projection matrix */
	SG_Real wPixel, hPixel;			/* Display pixel ratio */

	AG_Event *predraw_ev;			/* Pre-draw callback */
	AG_Event *postdraw_ev;			/* Post-draw callback */
	AG_Event *scale_ev;			/* Scaling/movement event */
	AG_Event *keydown_ev, *keyup_ev;	/* Keyboard events */
	AG_Event *btndown_ev, *btnup_ev;	/* Mouse button events */
	AG_Event *motion_ev;			/* Mouse motion event */

	char status[128];			/* Status text buffer */
	struct {
		SG_Vector last;			/* Last coordinates */
		int panning;			/* Panning mode */
	} mouse;
	SK_Tool *curtool;			/* Selected tool */
	SK_Tool *deftool;			/* Default tool if any */
	TAILQ_HEAD(, sk_tool) tools;		/* Sketching tools */
} SK_View;

#define SKVIEW(p) ((SK_View *)(p))

__BEGIN_DECLS
SK_View	  *SK_ViewNew(void *, struct sk *, Uint);
void	   SK_ViewInit(SK_View *, struct sk *, Uint);
void	   SK_ViewDestroy(void *);
void	   SK_ViewDraw(void *);
void	   SK_ViewScale(void *, int, int);
void	   SK_ViewZoom(SK_View *, SG_Real);
void	   SK_ViewReshape(SK_View *);

void	   SK_ViewPreDrawFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewPostDrawFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewScaleFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewKeydownFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewKeyupFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewButtondownFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewButtonupFn(SK_View *, AG_EventFn, const char *, ...);
void	   SK_ViewMotionFn(SK_View *, AG_EventFn, const char *, ...);

void	   SK_ViewSelectTool(SK_View *, SK_Tool *, void *);
SK_Tool	  *SK_ViewFindTool(SK_View *, const char *);
SK_Tool	  *SK_ViewFindToolByOps(SK_View *, const SK_ToolOps *);
SK_Tool	  *SK_ViewRegTool(SK_View *, const SK_ToolOps *, void *);
void	   SK_ViewSetDefaultTool(SK_View *, SK_Tool *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SK_VIEW_H_ */
