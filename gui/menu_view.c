/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "menu.h"
#include "primitive.h"
#include "icons.h"

/* Widget must be locked. */
static void
SelectItem(AG_MenuItem *pitem, AG_MenuItem *subitem)
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
SubmenuTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *item = arg;
	AG_Menu *m = mview->pmenu;

#ifdef DEBUG
	if (item != mview->pitem->sel_subitem)
		AG_FatalError("AG_Menu: Subitem mismatch in timeout");
#endif
	AG_MenuExpand(m, item, WIDGET(mview)->cx2, 
	                     WIDGET(mview)->cy+item->y);
	return (0);
}

static void
MouseMotion(AG_Event *event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(1);
	int my = AG_INT(2);
	int y = mview->tPad, i;

	if (my < 0)
		return;
	if (mx < 0 || mx > WIDGET(mview)->w)
		goto selnone;

	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *subitem = &pitem->subitems[i];

		AG_MenuUpdateItem(subitem);

		y += m->itemh;
		if (my < y) {
			if (mx > WIDGET(mview)->w &&
			    subitem->nsubitems == 0) {
				goto selnone;
			}
			if (pitem->sel_subitem != subitem &&
			    (subitem->flags & AG_MENU_ITEM_NOSELECT) == 0) {
				SelectItem(pitem, subitem);
			}
			return;
		}
	}
selnone:
	if (pitem->sel_subitem != NULL &&
	    pitem->sel_subitem->nsubitems == 0)
		SelectItem(pitem, NULL);
}

static int
GetItemBoolValue(AG_MenuItem *mi)
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
SetItemBoolValue(AG_MenuItem *mi)
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
MouseButtonUp(AG_Event *event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(2);
	int my = AG_INT(3);
	int y = mview->tPad;
	int i;

	if (my < 0 || mx < 0) {
		goto collapse;
	}
	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *item = &pitem->subitems[i];

		AG_MenuUpdateItem(item);

		y += m->itemh;
		if (my < y && mx >= 0 && mx <= WIDGET(mview)->w) {
			if (item->state == 0) {
				goto collapse;
			}
			if (item->clickFn != NULL) {
				AG_ExecEventFn(m, item->clickFn);
			}
			if (item->bind_type != AG_MENU_NO_BINDING) {
				if (item->bind_lock != NULL)
					AG_MutexLock(item->bind_lock);

				SetItemBoolValue(item);

				if (item->bind_lock != NULL)
					AG_MutexUnlock(item->bind_lock);
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
	SelectItem(pitem, NULL);
	m->itemSel = NULL;
	m->selecting = 0;
}

static void
Init(void *obj)
{
	AG_MenuView *mview = obj;

	WIDGET(mview)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                        AG_WIDGET_UNFOCUSED_BUTTONUP;

	mview->panel = NULL;
	mview->pmenu = NULL;
	mview->pitem = NULL;
	mview->spIconLbl = 8;
	mview->spLblArrow = 16;
	mview->lPad = 8;
	mview->rPad = 8;
	mview->tPad = 4;
	mview->bPad = 4;

	AG_SetEvent(mview, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(mview, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetTimeout(&mview->submenu_to, SubmenuTimeout, NULL, 0);

	/* XXX wasteful */
	AG_WidgetMapSurface(mview, AG_DupSurface(agIconSmallArrowRight.s));
}

static void
Draw(void *p)
{
	AG_MenuView *mview = p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	AG_Rect r;
	int i;

	STYLE(mview)->MenuBackground(mview,
	    AG_RECT(0, 0, WIDTH(mview), HEIGHT(mview)));

	r.x = 0;
	r.y = mview->tPad;
	r.w = WIDGET(mview)->w;
	r.h = m->itemh;

	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *item = &pitem->subitems[i];
		int x = mview->lPad;
		
		AG_MenuUpdateItem(item);

		if (item->icon == -1 && item->iconSrc != NULL) {
			item->icon = AG_WidgetMapSurface(m, item->iconSrc);
		}
		STYLE(mview)->MenuItemBackground(mview, r, x, m, item->icon,
		    (item == pitem->sel_subitem && item->state == 1),
		    (item->value != -1) ? item->value : GetItemBoolValue(item));

		if (pitem->flags & AG_MENU_ITEM_ICONS) {
			x += m->itemh + mview->spIconLbl;
		}
		if (item->flags & AG_MENU_ITEM_SEPARATOR) {
			STYLE(mview)->MenuItemSeparator(mview,
			    mview->lPad,
			    WIDTH(mview) - mview->rPad - 1,
			    r.y, m->itemh);
		} else {
			int lbl = item->state ? item->lblEnabled :
			                        item->lblDisabled;

			if (item->state == 1) {
				if (item->lblEnabled == -1) {
					AG_TextColor(MENU_TXT_COLOR);
					item->lblEnabled =
					    (item->text == NULL) ? -1 :
					    AG_WidgetMapSurface(m,
					    AG_TextRender(item->text));
				}
				lbl = item->lblEnabled;
			} else {
				if (item->lblDisabled == -1) {
					AG_TextColor(MENU_TXT_DISABLED_COLOR);
					item->lblDisabled =
					    (item->text == NULL) ? -1 :
					    AG_WidgetMapSurface(m,
					    AG_TextRender(item->text));
				}
				lbl = item->lblDisabled;
			}
			AG_WidgetBlitFrom(mview, m, lbl, NULL,
			    x,
			    r.y + m->itemh/2 - WSURFACE(m,lbl)->h/2 + 1);
			x += WSURFACE(m,lbl)->w;
		}
		if (item->nsubitems > 0) {
			x += mview->spLblArrow;
			AG_WidgetBlitSurface(mview, 0,
			    x,
			    r.y + m->itemh/2 - agIconSmallArrowRight.s->h/2 -1);
		}
		r.y += m->itemh;
	}
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_MenuView *mview = p;
	AG_MenuItem *pitem = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int i;

	r->w = 0;
	r->h = mview->tPad + mview->bPad;
		
	for (i = 0; i < pitem->nsubitems; i++) {
		AG_MenuItem *item = &pitem->subitems[i];
		int wReq = mview->lPad + mview->rPad;
		int wLbl;
	
		AG_MenuUpdateItem(item);

		if (item->icon != -1) {
			wReq += WSURFACE(m,item->icon)->w;
		}
		if (pitem->flags & AG_MENU_ITEM_ICONS)
			wReq += m->itemh + mview->spIconLbl;
	
		if (item->lblEnabled != -1) {
			wLbl = WSURFACE(m,item->lblEnabled)->w;
		} else if (item->lblDisabled != -1) {
			wLbl = WSURFACE(m,item->lblDisabled)->w;
		} else {
			AG_TextSize(item->text, &wLbl, NULL);
		}
		wReq += wLbl;

		if (item->nsubitems > 0) {
			wReq += mview->spLblArrow + agIconSmallArrowRight.s->w;
		}
		if (wReq > r->w) {
			r->w = wReq;
		}
		r->h += m->itemh;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	return (0);
}

AG_WidgetClass agMenuViewClass = {
	{
		"AG_Widget:AG_MenuView",
		sizeof(AG_MenuView),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
