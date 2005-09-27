/*	$Csoft: menu_view.c,v 1.25 2005/09/01 02:38:32 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <engine/engine.h>
#include <engine/view.h>

#include "menu.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static AG_WidgetOps menu_view_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_WidgetDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_MenuViewDraw,
	AG_MenuViewScale
};

static void
select_subitem(AG_MenuItem *pitem, AG_MenuItem *subitem)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuView *mview = pitem->view;

	if (pitem->sel_subitem != NULL &&
	    pitem->sel_subitem->view != NULL) {
		AG_MenuCollapse(m, pitem->sel_subitem);
	}
	pitem->sel_subitem = subitem;

	AG_LockTimeouts(m);
	AG_DelTimeout(mview, &mview->submenu_to);
	if (subitem != NULL &&
	    subitem->nsubitems > 0) {
		AG_AddTimeout(mview, &mview->submenu_to, 200);
		mview->submenu_to.arg = subitem;
	}
	AG_UnlockTimeouts(m);
}

static Uint32
submenu_timeout(void *obj, Uint32 ival, void *arg)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *item = arg;
	AG_Menu *m = mview->pmenu;

#ifdef DEBUG
	if (item != mview->pitem->sel_subitem)
		fatal("subitem");
#endif
	AG_MenuExpand(m, item, AGWIDGET(mview)->cx2, 
	                     AGWIDGET(mview)->cy+item->y);
	return (0);
}

static void
mousemotion(int argc, union evarg *argv)
{
	AG_MenuView *mview = argv[0].p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = argv[1].i;
	int my = argv[2].i;
	int y = mview->vpadding, i;

	if (my < 0)
		return;
	if (mx < 0)
		goto selnone;

	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *subitem = &pitem->subitems[i];

		y += m->itemh;
		if (my < y) {
			if (mx > AGWIDGET(mview)->w &&
			    subitem->nsubitems == 0) {
				goto selnone;
			}
			if (pitem->sel_subitem != subitem) {
				select_subitem(pitem, subitem);
			}
			return;
		}
	}
selnone:
	if (pitem->sel_subitem != NULL &&
	    pitem->sel_subitem->nsubitems == 0) {
		select_subitem(pitem, NULL);
	}
}

static int
get_option(AG_MenuItem *mi)
{
	int val = 0;

	switch (mi->bind_type) {
	case AG_MENU_INT_BOOL:
		val = *(int *)mi->bind_p;
		break;
	case AG_MENU_INT8_BOOL:
		val = *(Uint8 *)mi->bind_p;
		break;
	case AG_MENU_INT_FLAGS:
		val = *(int *)mi->bind_p & mi->bind_flags;
		break;
	case AG_MENU_INT8_FLAGS:
		val = *(Uint8 *)mi->bind_p & mi->bind_flags;
		break;
	case AG_MENU_INT16_FLAGS:
		val = *(Uint16 *)mi->bind_p & mi->bind_flags;
		break;
	case AG_MENU_INT32_FLAGS:
		val = *(Uint32 *)mi->bind_p & mi->bind_flags;
		break;
	default:
		break;
	}
	return (mi->bind_invert ? !val : val);
}

static void
toggle_option(AG_MenuItem *mi)
{
	switch (mi->bind_type) {
	case AG_MENU_NO_BINDING:
		break;
	case AG_MENU_INT_BOOL:
		{
			int *boolp = (int *)mi->bind_p;

			*boolp = !(*boolp);
		}
		break;
	case AG_MENU_INT8_BOOL:
		{
			Uint8 *boolp = (Uint8 *) mi->bind_p;

			*boolp = !(*boolp);
		}
		break;
	case AG_MENU_INT_FLAGS:
		{
			int *flags = (int *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case AG_MENU_INT8_FLAGS:
		{
			Uint8 *flags = (Uint8 *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case AG_MENU_INT16_FLAGS:
		{
			Uint16 *flags = (Uint16 *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case AG_MENU_INT32_FLAGS:
		{
			Uint32 *flags = (Uint32 *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	AG_MenuView *mview = argv[0].p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = argv[2].i;
	int my = argv[3].i;
	int y = mview->vpadding;
	int i;

	if (my < 0 || mx < 0) {
		goto collapse;
	}
	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *mi = &pitem->subitems[i];

		y += m->itemh;
		if (my < y && mx >= 0 && mx <= AGWIDGET(mview)->w) {
			if (mi->onclick != NULL) {
				AG_PostEvent(NULL, m, mi->onclick->name, NULL);
			}
			if (mi->bind_type != AG_MENU_NO_BINDING) {
				if (mi->bind_lock != NULL)
					pthread_mutex_lock(mi->bind_lock);

				toggle_option(mi);

				if (mi->bind_lock != NULL)
					pthread_mutex_unlock(mi->bind_lock);
			}
			goto collapse;
		}
	}
	if (i == pitem->nsubitems) {
		goto collapse;
	}
	return;
collapse:
	AG_MenuCollapse(m, pitem);
	select_subitem(pitem, NULL);
	m->sel_item = NULL;
	m->selecting = 0;
}

void
AG_MenuViewInit(void *p, AG_Window *panel, AG_Menu *pmenu,
    AG_MenuItem *pitem)
{
	AG_MenuView *mview = p;

	AG_WidgetInit(mview, "AGMenuView", &menu_view_ops,
	    AG_WIDGET_UNFOCUSED_MOTION|AG_WIDGET_UNFOCUSED_BUTTONUP);
	AG_WireGfx(mview, "/engine/widget/pixmaps");

	mview->panel = panel;
	mview->pmenu = pmenu;
	mview->pitem = pitem;
	mview->hspace = 6;
	mview->vpadding = 4;

	/* XXX */
	AG_WidgetMapSurface(mview, AG_DupSurface(AG_SPRITE(mview,3).su));
	
	AG_SetEvent(mview, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(mview, "window-mousebuttonup", mousebuttonup, NULL);

	AG_SetTimeout(&mview->submenu_to, submenu_timeout, NULL, 0);
}

#define VERT_ALIGNED(m, h) ((m)->itemh/2 - (h)/2 - 1)

void
AG_MenuViewDraw(void *p)
{
	AG_MenuView *mview = p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int i, y = mview->vpadding;
	
	agPrim.box(mview, 0, 0, AGWIDGET(mview)->w, AGWIDGET(mview)->h, 1,
	    AG_COLOR(MENU_UNSEL_COLOR));

	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *subitem = &pitem->subitems[i];
		int x = mview->hspace;
		
		if (subitem == pitem->sel_subitem) {
			agPrim.rect_filled(mview,
			    1, 1+y,
			    AGWIDGET(mview)->w - 2,
			    m->itemh,
			    AG_COLOR(MENU_SEL_COLOR));
		}
		
		if (subitem->poll != NULL)
			AG_PostEvent(NULL, m, subitem->poll->name, "%p",
			    subitem);

		if (subitem->icon != -1) {
			SDL_Surface *iconsu = AGWIDGET_SURFACE(m,subitem->icon);
			int dy = VERT_ALIGNED(m, iconsu->h);
			int state;

			AG_WidgetBlitFrom(mview, m, subitem->icon, NULL,
			    x+(m->itemh/2 - iconsu->w/2), y+dy+2);

			state = (subitem->state != -1) ? subitem->state :
			    get_option(subitem);

			if (state) {
				Uint8 c[4];

				SDL_GetRGB(AG_COLOR(MENU_OPTION_COLOR),
				    agVideoFmt,
				    &c[0], &c[1], &c[2]);
				c[3] = 64;
				agPrim.frame(mview, x, y+2,
				    m->itemh, m->itemh-2,
				    AG_COLOR(MENU_OPTION_COLOR));
				agPrim.rect_blended(mview, x, y+2,
				    m->itemh, m->itemh-2, c, AG_ALPHA_SRC);
			}
		}
		
		x = m->itemh + mview->hspace*2;

		if (subitem->label != -1) {
			SDL_Surface *lbl = AGWIDGET_SURFACE(m,subitem->label);

			AG_WidgetBlitFrom(mview, m, subitem->label, NULL,
			    x, y+VERT_ALIGNED(m, lbl->h));
			x += lbl->w + mview->hspace;
		} else {
			int dy = m->itemh/2 - 1;
			int dx = AGWIDGET(mview)->w - mview->hspace - 1;

			agPrim.hline(mview,
			    mview->hspace,
			    dx,
			    y+dy,
			    AG_COLOR(MENU_SEP1_COLOR));
			agPrim.hline(mview,
			    mview->hspace,
			    dx,
			    y+dy+1,
			    AG_COLOR(MENU_SEP2_COLOR));
		}

		if (subitem->nsubitems > 0) {
			AG_WidgetBlitSurface(mview, 0,
			    x,
			    y + m->itemh/2 - AGWIDGET_SURFACE(mview,0)->h/2 -1);
		}
		y += m->itemh;
	}
}

void
AG_MenuViewScale(void *p, int w, int h)
{
	AG_MenuView *mview = p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int i;

	if (w == -1 && h == -1) {
		AGWIDGET(mview)->w = 0;
		AGWIDGET(mview)->h = mview->vpadding*2;
		
		for (i = 0; i < pitem->nsubitems; i++) {
			AG_MenuItem *subitem = &pitem->subitems[i];
			int req_w = mview->hspace*2;

			if (subitem->icon != -1) {
				req_w += AGWIDGET_SURFACE(m,subitem->icon)->w;
			}
			req_w += m->itemh;
		
			if (subitem->label != -1) {
				req_w += AGWIDGET_SURFACE(m,subitem->label)->w +
				    mview->hspace;
			}
			if (subitem->nsubitems > 0) {
				req_w += AGWIDGET_SURFACE(mview,0)->w +
				    mview->hspace;
			}
			if (req_w > AGWIDGET(mview)->w) {
				AGWIDGET(mview)->w = req_w;
			}
			AGWIDGET(mview)->h += m->itemh;
		}
	}
}

