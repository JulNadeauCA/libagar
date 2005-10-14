/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SG_VIEW_H_
#define _AGAR_SG_VIEW_H_

#include <agar/gui/glview.h>

#include "begin_code.h"

typedef struct sg_view {
	struct ag_glview glv;
	u_int flags;
#define SG_VIEW_WFILL	0x01
#define SG_VIEW_HFILL	0x02
	SG *sg;
} SG_View;

__BEGIN_DECLS
SG_View	*SG_ViewNew(void *, SG *, u_int);
void	 SG_ViewInit(SG_View *, SG *, u_int);
void	 SG_ViewDestroy(void *);

void	 SG_ViewReshape(SG_View *);
void	 SG_ViewKeydownFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewKeyupFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewButtondownFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewButtonupFn(SG_View *, AG_EventFn, const char *, ...);
void	 SG_ViewMotionFn(SG_View *, AG_EventFn, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SG_GLVIEW_H_ */
