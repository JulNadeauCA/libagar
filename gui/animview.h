/*	$Csoft: animview.h,v 1.1 2005/03/24 04:02:07 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_ANIMVIEW_H_
#define _AGAR_WIDGET_ANIMVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/window.h>
#include <agar/rg/animation.h>

#include "begin_code.h"

/* XXX move to librg */

typedef struct rg_anim_view {
	struct ag_widget wid;
	RG_Anim *anim;
	float speed;				/* Delay multiplier */
	Uint frame;				/* Current frame */
	Uint pre_w, pre_h;			/* Prescale geometry */
	SDL_Rect ranim;				/* Preview rectangle */
	struct {
		AG_Button *play;
		AG_Button *pause;
		AG_Button *stop;
	} btns;
	AG_Timeout timer;			/* Processing timer */
	AG_Menu *menu;			/* Popup menu */
	AG_MenuItem *menu_item;
	AG_Window *menu_win;
} RG_Animview;

__BEGIN_DECLS
RG_Animview	*RG_AnimviewNew(void *);
void		 RG_AnimviewInit(RG_Animview *);
void		 RG_AnimviewPrescale(RG_Animview *, int, int);
void		 RG_AnimviewDestroy(void *);
void		 RG_AnimviewDraw(void *);
void		 RG_AnimviewScale(void *, int, int);
void		 RG_AnimviewSetAnimation(RG_Animview *, RG_Anim *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_ANIMVIEW_H */
