/*	$Csoft: animview.h,v 1.10 2005/01/28 12:49:51 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_ANIMVIEW_H_
#define _AGAR_WIDGET_ANIMVIEW_H_

#include <engine/widget/widget.h>
#include <engine/widget/button.h>
#include <engine/widget/menu.h>
#include <engine/widget/window.h>
#include <engine/rg/animation.h>

#include "begin_code.h"

struct animview {
	struct widget wid;
	struct animation *anim;
	float speed;				/* Delay multiplier */
	u_int frame;				/* Current frame */
	u_int pre_w, pre_h;			/* Prescale geometry */
	SDL_Rect ranim;				/* Preview rectangle */
	struct {
		struct button *play;
		struct button *pause;
		struct button *stop;
	} btns;
	struct timeout timer;			/* Processing timer */
	struct AGMenu *menu;			/* Popup menu */
	struct AGMenuItem *menu_item;
	struct window *menu_win;
};

__BEGIN_DECLS
struct animview	*animview_new(void *);
void		 animview_init(struct animview *);
void		 animview_prescale(struct animview *, int, int);
void		 animview_destroy(void *);
void		 animview_draw(void *);
void		 animview_scale(void *, int, int);
void		 animview_set_animation(struct animview *, struct animation *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_ANIMVIEW_H */
