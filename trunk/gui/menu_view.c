/*
 * Copyright (c) 2004-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Sub-item selection has moved over the specified subitem. If the subitem is
 * a submenu, the submenu timer is initiated.
 */
static void
SelectItem(AG_MenuItem *mi, AG_MenuItem *subitem)
{
	AG_Menu *m = mi->pmenu;

	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->view != NULL) {
		AG_MenuCollapse(m, mi->sel_subitem);
	}
	mi->sel_subitem = subitem;

	if (subitem != NULL) {
		AG_MenuView *mview = mi->view;

		AG_LockTimeouts(m);
		AG_DelTimeout(mview, &mview->submenu_to);
		if (subitem != NULL &&
		    subitem->nsubitems > 0) {
			AG_ScheduleTimeout(mview, &mview->submenu_to, 200);
			mview->submenu_to.arg = subitem;
		}
		AG_UnlockTimeouts(m);
	}
}

static Uint32
SubmenuTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *item = arg;

#ifdef AG_DEBUG
	if (item != mview->pitem->sel_subitem)
		AG_FatalError("AG_Menu: Subitem mismatch in timeout");
#endif
	AG_MenuExpand(mview, item, WIDTH(mview), item->y);
	return (0);
}

static void
MouseMotion(AG_Event *event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *mi = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(1);
	int my = AG_INT(2);
	int y = mview->tPad, i;

	if (my < 0)
		return;
	if (mx < 0 || mx > WIDTH(mview))
		goto selnone;

	for (i = 0; i < mi->nsubitems; i++) {
		AG_MenuItem *subitem = &mi->subitems[i];

		AG_MenuUpdateItem(subitem);

		y += m->itemh;
		if (my < y) {
			if (mx > WIDTH(mview) &&
			    subitem->nsubitems == 0) {
				goto selnone;
			}
			if (subitem->flags & AG_MENU_ITEM_SEPARATOR) {
				goto selnone;
			}
			if (mi->sel_subitem != subitem &&
			    (subitem->flags & AG_MENU_ITEM_NOSELECT) == 0) {
				SelectItem(mi, subitem);
			}
			AG_Redraw(mview);
			return;
		}
	}
selnone:
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->nsubitems == 0) {
		AG_Redraw(mview);
		SelectItem(mi, NULL);
	}
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
			AG_INVFLAGS(*flags, mi->bind_flags);
		}
		break;
	case AG_MENU_INT8_FLAGS:
		{
			Uint8 *flags = (Uint8 *)mi->bind_p;
			AG_INVFLAGS(*flags, mi->bind_flags);
		}
		break;
	case AG_MENU_INT16_FLAGS:
		{
			Uint16 *flags = (Uint16 *)mi->bind_p;
			AG_INVFLAGS(*flags, mi->bind_flags);
		}
		break;
	case AG_MENU_INT32_FLAGS:
		{
			Uint32 *flags = (Uint32 *)mi->bind_p;
			AG_INVFLAGS(*flags, mi->bind_flags);
		}
		break;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *mi = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(2);
	int my = AG_INT(3);
	int y = mview->tPad;
	Uint i;

	if (mx < 0 || mx >= WIDTH(mview) ||
	    my < 0 || my >= HEIGHT(mview)) {
		return;
	}
	for (i = 0; i < mi->nsubitems; i++) {
		AG_MenuItem *item = &mi->subitems[i];

		AG_MenuUpdateItem(item);

		y += m->itemh;
		if (my < y && mx >= 0 && mx <= WIDTH(mview)) {
			if (item->state == 0) {
				/* Nothing to do */
			} else if (item->clickFn != NULL) {
				AG_ExecEventFn(m, item->clickFn);
				AG_MenuCollapseAll(m);
			} else if (item->bind_type != AG_MENU_NO_BINDING) {
				if (item->bind_lock != NULL) {
					AG_MutexLock(item->bind_lock);
				}
				SetItemBoolValue(item);
				if (item->bind_lock != NULL) {
					AG_MutexUnlock(item->bind_lock);
				}
				AG_MenuCollapseAll(m);
			}
			AG_Redraw(mview);
			return;
		}
	}
}

static void
Shown(AG_Event *event)
{
	AG_MenuView *mview = AG_SELF();

	/* XXX wasteful */
	if (mview->arrowRight == -1) {
		mview->arrowRight = AG_WidgetMapSurface(mview,
		    AG_SurfaceDup(agIconSmallArrowRight.s));
	}
}

static void
Init(void *obj)
{
	AG_MenuView *mview = obj;

	WIDGET(mview)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                        AG_WIDGET_UNFOCUSED_BUTTONUP;

	mview->pmenu = NULL;
	mview->pitem = NULL;
	mview->spIconLbl = 8;
	mview->spLblArrow = 16;
	mview->lPad = 8;
	mview->rPad = 8;
	mview->tPad = 4;
	mview->bPad = 4;
	mview->arrowRight = -1;

	AG_AddEvent(mview, "widget-shown", Shown, NULL);
	AG_SetEvent(mview, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(mview, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetTimeout(&mview->submenu_to, SubmenuTimeout, NULL, 0);

#ifdef AG_DEBUG
	AG_BindInt(mview, "spIconLbl", &mview->spIconLbl);
	AG_BindInt(mview, "spLblArrow", &mview->spLblArrow);
	AG_BindInt(mview, "lPad", &mview->lPad);
	AG_BindInt(mview, "rPad", &mview->rPad);
	AG_BindInt(mview, "tPad", &mview->tPad);
	AG_BindInt(mview, "bPad", &mview->bPad);
#endif
}

static void
Draw(void *obj)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *mi = mview->pitem;
	AG_Menu *m = mview->pmenu;
	AG_Rect r;
	int i;

	STYLE(mview)->MenuBackground(mview,
	    AG_RECT(0, 0, WIDTH(mview), HEIGHT(mview)));

	r.x = 0;
	r.y = mview->tPad;
	r.w = WIDTH(mview);
	r.h = m->itemh;

	for (i = 0; i < mi->nsubitems; i++) {
		AG_MenuItem *item = &mi->subitems[i];
		int x = mview->lPad;
		AG_Color C;
		int boolState;

		/* Update dynamic item if needed. */
		AG_MenuUpdateItem(item);

		/* Indicate active item selection */
		if (item == mi->sel_subitem && item->state == 1)
			AG_DrawRect(mview, r, agColors[MENU_SEL_COLOR]);

		/* Render the menu item's icon */
		if (item->icon == -1 &&
		    item->iconSrc != NULL) {
			item->icon = AG_WidgetMapSurface(mview,
			    AG_SurfaceDup(item->iconSrc));
		}
		if (item->icon != -1) {
			AG_WidgetBlitSurface(mview, item->icon,
			    x   + (r.h/2 - item->iconSrc->w/2),
			    r.y + (r.h/2 - item->iconSrc->h/2) + 1);

			/* Indicate boolean state */
			boolState = (item->value != -1) ? item->value :
			            GetItemBoolValue(item);
			if (boolState) {
				AG_DrawFrame(mview,
				    AG_RECT(x, r.y+2, r.h, r.h-2),
				    1, agColors[MENU_OPTION_COLOR]);
				C = agColors[MENU_OPTION_COLOR];
				C.a = 64;
				AG_DrawRectBlended(mview,
				    AG_RECT(x, r.y+2, r.h, r.h-2),
				    C, AG_ALPHA_SRC);
			}
		}

		/* Keep columns aligned if there are icons. */
		if (mi->flags & AG_MENU_ITEM_ICONS)
			x += m->itemh + mview->spIconLbl;

		if (item->flags & AG_MENU_ITEM_SEPARATOR) {
			/* Render menu separator item */
			STYLE(mview)->MenuItemSeparator(mview,
			    mview->lPad,
			    WIDTH(mview) - mview->rPad - 1,
			    r.y, m->itemh);
		} else {
			int lbl = item->state ? item->lblView[1] :
			                        item->lblView[0];

			/* Render the menu item's text string */
			if (item->state == 1) {
				if (item->lblView[1] == -1) {
					AG_TextColor(agColors[MENU_TXT_COLOR]);
					item->lblView[1] =
					    (item->text == NULL) ? -1 :
					    AG_WidgetMapSurface(mview,
					    AG_TextRender(item->text));
				}
				lbl = item->lblView[1];
			} else {
				if (item->lblView[0] == -1) {
					AG_TextColor(agColors[MENU_TXT_DISABLED_COLOR]);
					item->lblView[0] =
					    (item->text == NULL) ? -1 :
					    AG_WidgetMapSurface(mview,
					    AG_TextRender(item->text));
				}
				lbl = item->lblView[0];
			}
			AG_WidgetBlitSurface(mview, lbl,
			    x,
			    r.y + m->itemh/2 - agTextFontHeight/2 + 1);
			x += WSURFACE(mview,lbl)->w;
		}

		/* Render the submenu arrow. */
		if (item->nsubitems > 0) {
			x += mview->spLblArrow;
			AG_WidgetBlitSurface(mview, mview->arrowRight,
			    x,
			    r.y + m->itemh/2 - agIconSmallArrowRight.s->h/2 -1);
		}

		r.y += m->itemh;
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *mi = mview->pitem;
	AG_Menu *m = mview->pmenu;
	int i;

	r->w = 0;
	r->h = mview->tPad + mview->bPad;
		
	for (i = 0; i < mi->nsubitems; i++) {
		AG_MenuItem *item = &mi->subitems[i];
		int wReq = mview->lPad + mview->rPad;
		int wLbl;
	
		AG_MenuUpdateItem(item);
		
		if (item->icon != -1) {
			wReq += item->iconSrc->w;
		}
		if (mi->flags & AG_MENU_ITEM_ICONS)
			wReq += m->itemh + mview->spIconLbl;
	
		AG_TextSize(item->text, &wLbl, NULL);
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
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	return (0);
}

AG_WidgetClass agMenuViewClass = {
	{
		"Agar(Widget:MenuView)",
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
