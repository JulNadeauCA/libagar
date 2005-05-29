/*	$Csoft: menu_view.c,v 1.20 2005/05/24 05:06:17 vedge Exp $	*/

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

static struct widget_ops menu_view_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		widget_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	menu_view_draw,
	menu_view_scale
};

static void
select_subitem(struct AGMenuItem *pitem, struct AGMenuItem *subitem)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuView *mview = pitem->view;

	if (pitem->sel_subitem != NULL &&
	    pitem->sel_subitem->view != NULL) {
		menu_collapse(m, pitem->sel_subitem);
	}
	pitem->sel_subitem = subitem;

	lock_timeout(m);
	timeout_del(mview, &mview->submenu_to);
	if (subitem != NULL &&
	    subitem->nsubitems > 0) {
		timeout_add(mview, &mview->submenu_to, 200);
		mview->submenu_to.arg = subitem;
	}
	unlock_timeout(m);
}

static Uint32
submenu_timeout(void *obj, Uint32 ival, void *arg)
{
	struct AGMenuView *mview = obj;
	struct AGMenuItem *item = arg;
	struct AGMenu *m = mview->pmenu;

#ifdef DEBUG
	if (item != mview->pitem->sel_subitem)
		fatal("subitem");
#endif
	menu_expand(m, item, WIDGET(mview)->cx2, 
	                     WIDGET(mview)->cy+item->y);
	return (0);
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct AGMenuView *mview = argv[0].p;
	struct AGMenuItem *pitem = mview->pitem;
	struct AGMenu *m = mview->pmenu;
	int mx = argv[1].i;
	int my = argv[2].i;
	int y = mview->vpadding, i;

	if (my < 0)
		return;
	if (mx < 0)
		goto selnone;

	for (i = 0; i < pitem->nsubitems; i++) {
		struct AGMenuItem *subitem = &pitem->subitems[i];

		y += m->itemh;
		if (my < y) {
			if (mx > WIDGET(mview)->w &&
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
get_option(struct AGMenuItem *mi)
{
	int val = 0;

	switch (mi->bind_type) {
	case MENU_INT_BOOL:
		val = *(int *)mi->bind_p;
		break;
	case MENU_INT8_BOOL:
		val = *(Uint8 *)mi->bind_p;
		break;
	case MENU_INT_FLAGS:
		val = *(int *)mi->bind_p & mi->bind_flags;
		break;
	case MENU_INT8_FLAGS:
		val = *(Uint8 *)mi->bind_p & mi->bind_flags;
		break;
	case MENU_INT16_FLAGS:
		val = *(Uint16 *)mi->bind_p & mi->bind_flags;
		break;
	case MENU_INT32_FLAGS:
		val = *(Uint32 *)mi->bind_p & mi->bind_flags;
		break;
	default:
		break;
	}
	return (mi->bind_invert ? !val : val);
}

static void
toggle_option(struct AGMenuItem *mi)
{
	switch (mi->bind_type) {
	case MENU_NO_BINDING:
		break;
	case MENU_INT_BOOL:
		{
			int *boolp = (int *)mi->bind_p;

			*boolp = !(*boolp);
		}
		break;
	case MENU_INT8_BOOL:
		{
			Uint8 *boolp = (Uint8 *) mi->bind_p;

			*boolp = !(*boolp);
		}
		break;
	case MENU_INT_FLAGS:
		{
			int *flags = (int *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case MENU_INT8_FLAGS:
		{
			Uint8 *flags = (Uint8 *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case MENU_INT16_FLAGS:
		{
			Uint16 *flags = (Uint16 *)mi->bind_p;

			if (*flags & mi->bind_flags) {
				*flags &= ~(mi->bind_flags);
			} else {
				*flags |= mi->bind_flags;
			}
		}
		break;
	case MENU_INT32_FLAGS:
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
	struct AGMenuView *mview = argv[0].p;
	struct AGMenuItem *pitem = mview->pitem;
	struct AGMenu *m = mview->pmenu;
	int mx = argv[2].i;
	int my = argv[3].i;
	int y = mview->vpadding;
	int i;

	if (my < 0 || mx < 0) {
		goto collapse;
	}
	for (i = 0; i < pitem->nsubitems; i++) {
		struct AGMenuItem *mi = &pitem->subitems[i];

		y += m->itemh;
		if (my < y && mx >= 0 && mx <= WIDGET(mview)->w) {
			if (mi->onclick != NULL) {
				event_post(NULL, m, mi->onclick->name, NULL);
			}
			if (mi->bind_type != MENU_NO_BINDING) {
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
	menu_collapse(m, pitem);
	select_subitem(pitem, NULL);
	m->sel_item = NULL;
	m->selecting = 0;
}

void
menu_view_init(void *p, struct window *panel, struct AGMenu *pmenu,
    struct AGMenuItem *pitem)
{
	struct AGMenuView *mview = p;

	widget_init(mview, "AGMenuView", &menu_view_ops,
	    WIDGET_UNFOCUSED_MOTION|WIDGET_UNFOCUSED_BUTTONUP);
	object_wire_gfx(mview, "/engine/widget/pixmaps");

	mview->panel = panel;
	mview->pmenu = pmenu;
	mview->pitem = pitem;
	mview->hspace = 6;
	mview->vpadding = 4;

	/* XXX */
	widget_map_surface(mview, view_copy_surface(SPRITE(mview,3).su));
	
	event_new(mview, "window-mousemotion", mousemotion, NULL);
	event_new(mview, "window-mousebuttonup", mousebuttonup, NULL);

	timeout_set(&mview->submenu_to, submenu_timeout, NULL, 0);
}

#define VERT_ALIGNED(m, h) ((m)->itemh/2 - (h)/2 - 1)

void
menu_view_draw(void *p)
{
	struct AGMenuView *mview = p;
	struct AGMenuItem *pitem = mview->pitem;
	struct AGMenu *m = mview->pmenu;
	int i, y = mview->vpadding;
	
	primitives.box(mview, 0, 0, WIDGET(mview)->w, WIDGET(mview)->h, 1,
	    COLOR(MENU_UNSEL_COLOR));

	for (i = 0; i < pitem->nsubitems; i++) {
		struct AGMenuItem *subitem = &pitem->subitems[i];
		int x = mview->hspace;
		
		if (subitem == pitem->sel_subitem) {
			primitives.rect_filled(mview,
			    1, 1+y,
			    WIDGET(mview)->w - 2,
			    m->itemh,
			    COLOR(MENU_SEL_COLOR));
		}

		if (subitem->icon != -1) {
			SDL_Surface *iconsu = WIDGET_SURFACE(m,subitem->icon);
			int dy = VERT_ALIGNED(m, iconsu->h);

			widget_blit_from(mview, m, subitem->icon, NULL,
			    x+(m->itemh/2 - iconsu->w/2), y+dy+2);

			if (get_option(subitem)) {
				Uint8 c[4];

				SDL_GetRGB(COLOR(MENU_OPTION_COLOR), vfmt,
				    &c[0], &c[1], &c[2]);
				c[3] = 128;
				primitives.frame(mview, x, y+2,
				    m->itemh, m->itemh-2,
				    COLOR(MENU_OPTION_COLOR));
				primitives.rect_blended(mview, x, y+2,
				    m->itemh, m->itemh-2, c, ALPHA_SRC);
			}
		}
		
		x = m->itemh + mview->hspace*2;

		if (subitem->label != -1) {
			SDL_Surface *lbl = WIDGET_SURFACE(m,subitem->label);

			widget_blit_from(mview, m, subitem->label, NULL,
			    x, y+VERT_ALIGNED(m, lbl->h));
			x += lbl->w + mview->hspace*2;
		} else {
			int dy = m->itemh/2 - 1;
			int dx = WIDGET(mview)->w - mview->hspace - 1;

			primitives.hline(mview,
			    mview->hspace,
			    dx,
			    y+dy,
			    COLOR(MENU_SEP1_COLOR));
			primitives.hline(mview,
			    mview->hspace,
			    dx,
			    y+dy+1,
			    COLOR(MENU_SEP2_COLOR));
		}

		if (subitem->nsubitems > 0) {
			widget_blit_surface(mview, 0,
			    x,
			    y + m->itemh/2 - WIDGET_SURFACE(mview,0)->h/2 - 1);
		}
		y += m->itemh;
	}
}

void
menu_view_scale(void *p, int w, int h)
{
	struct AGMenuView *mview = p;
	struct AGMenuItem *pitem = mview->pitem;
	struct AGMenu *m = mview->pmenu;
	int i;

	if (w == -1 && h == -1) {
		WIDGET(mview)->w = 0;
		WIDGET(mview)->h = mview->vpadding*2;
		
		for (i = 0; i < pitem->nsubitems; i++) {
			struct AGMenuItem *subitem = &pitem->subitems[i];
			int req_w = mview->hspace*2;

			if (subitem->icon != -1) {
				req_w += WIDGET_SURFACE(m,subitem->icon)->w;
			}
			req_w += m->itemh;
		
			if (subitem->label != -1) {
				req_w += WIDGET_SURFACE(m,subitem->label)->w +
				    mview->hspace;
			}
			if (subitem->nsubitems > 0) {
				req_w += WIDGET_SURFACE(mview,0)->w +
				    mview->hspace;
			}
			if (req_w > WIDGET(mview)->w) {
				WIDGET(mview)->w = req_w;
			}
			WIDGET(mview)->h += m->itemh;
		}
	}
}

