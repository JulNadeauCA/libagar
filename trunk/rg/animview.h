/*	Public domain	*/

#ifndef _AGAR_RG_ANIMVIEW_H_
#define _AGAR_RG_ANIMVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/window.h>
#include <agar/gui/iconmgr.h>

#include <agar/rg/tileset.h>

#include <agar/rg/begin.h>

typedef struct rg_anim_view {
	struct ag_widget wid;
	RG_Anim *anim;
	float speed;				/* Delay multiplier */
	Uint frame;				/* Current frame */
	Uint pre_w, pre_h;			/* SizeHint geometry */
	AG_Rect ranim;				/* Preview rectangle */
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
extern AG_WidgetClass rgAnimviewClass;

RG_Animview	*RG_AnimviewNew(void *);
void		 RG_AnimviewSizeHint(RG_Animview *, int, int);
void		 RG_AnimviewSetAnimation(RG_Animview *, RG_Anim *);
__END_DECLS

#include <agar/rg/close.h>
#endif /* _AGAR_RG_ANIMVIEW_H */
