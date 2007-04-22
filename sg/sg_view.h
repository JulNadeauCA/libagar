/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SG_VIEW_H_
#define _AGAR_SG_VIEW_H_

#include <agar/gui/glview.h>
#include <agar/gui/menu.h>

#include "begin_code.h"

struct sg;
struct sg_camera;

typedef struct sg_view {
	struct ag_glview glv;
	Uint flags;
#define SG_VIEW_HFILL		0x01
#define SG_VIEW_VFILL		0x02
#define SG_VIEW_NO_LIGHTING	0x04
#define SG_VIEW_NO_DEPTH_TEST	0x08
#define SG_VIEW_EXPAND		(SG_VIEW_HFILL|SG_VIEW_VFILL)
	struct sg *sg;			/* Scene graph */
	struct sg_camera *cam;		/* Current camera */
	struct {
		int lx, ly;		/* Last mouse position (for rotation) */
		SG_Vector rsens;	/* Rotation sensitivity vector */
		SG_Vector tsens;	/* Translation sensitivity vector */
	} mouse;
	AG_PopupMenu popup;		/* Popup menu for context */
	AG_Widget *editPane;		/* Edit container */
} SG_View;

__BEGIN_DECLS
SG_View	*SG_ViewNew(void *, struct sg *, Uint);
void	 SG_ViewInit(SG_View *, struct sg *, Uint);
void	 SG_ViewDestroy(void *);

void	 SG_ViewReshape(SG_View *);
void	 SG_ViewKeydownFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewKeyupFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewButtondownFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewButtonupFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewMotionFn(SG_View *, AG_EventFn, const char *, ...);

int	 SG_UnProject(SG_Real, SG_Real, SG_Real, const SG_Matrix *,
	              const SG_Matrix *, int *, SG_Vector *);
int	 SG_ViewUnProject(SG_View *, SG_Vector, SG_Real, SG_Real, SG_Vector *);
void	 SG_ViewUpdateProjection(SG_View *);

void	 SG_ViewSetCamera(SG_View *, struct sg_camera *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SG_GLVIEW_H_ */
