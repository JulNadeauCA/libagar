/*
 * Copyright (c) 2004-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/menu.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>

static Uint32
SubmenuTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *item = AG_PTR(1);

#ifdef AG_DEBUG
	if (item != mview->pitem->sel_subitem)
		AG_FatalError("AG_Menu: Subitem mismatch in timeout");
#endif
	AG_MenuExpand(mview, item, WIDTH(mview), item->y);
	return (0);
}

/*
 * Selection has moved over the specified subitem. If the subitem is
 * a submenu, the submenu timer is initiated.
 */
static void
SelectItem(AG_MenuItem *_Nonnull mi, AG_MenuItem *_Nullable subitem)
{
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->view != NULL) {
		AG_MenuCollapse(mi->sel_subitem);
	}
	mi->sel_subitem = subitem;

	if (subitem != NULL) {
		AG_MenuView *mview = mi->view;

		AG_LockTimers(mview);
		if (subitem != NULL && subitem->nSubItems > 0) {
			AG_AddTimer(mview, &mview->submenuTo, 200,
			    SubmenuTimeout, "%p", subitem);
		} else {
			AG_DelTimer(mview, &mview->submenuTo);
		}
		AG_UnlockTimers(mview);
	}
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *mi = mview->pitem, *miSub;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(1);
	int my = AG_INT(2);
	int y = mview->tPad;

	if (my < 0)
		return;
	if (mx < 0 || mx > WIDTH(mview))
		goto selnone;

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		AG_MenuUpdateItem(miSub);

		y += m->itemh;
		if (my < y) {
			if (mx > WIDTH(mview) &&
			    miSub->nSubItems == 0) {
				goto selnone;
			}
			if (miSub->flags & AG_MENU_ITEM_SEPARATOR) {
				goto selnone;
			}
			if (mi->sel_subitem != miSub &&
			    (miSub->flags & AG_MENU_ITEM_NOSELECT) == 0) {
				SelectItem(mi, miSub);
			}
			AG_Redraw(mview);
			return;
		}
	}
selnone:
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->nSubItems == 0) {
		AG_Redraw(mview);
		SelectItem(mi, NULL);
	}
}

static int
GetItemBoolValue(AG_MenuItem *_Nonnull mi)
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
SetItemBoolValue(AG_MenuItem *_Nonnull mi)
{
	switch (mi->bind_type) {
	case AG_MENU_INT_BOOL: {
		int *boolp = (int *)mi->bind_p;
		*boolp = !(*boolp);
		break;
	}
	case AG_MENU_INT8_BOOL: {
		Uint8 *boolp = (Uint8 *) mi->bind_p;
		*boolp = !(*boolp);
		break;
	}
	case AG_MENU_INT_FLAGS: {
		int *flags = (int *)mi->bind_p;
		AG_INVFLAGS(*flags, mi->bind_flags);
		break;
	}
	case AG_MENU_INT8_FLAGS: {
		Uint8 *flags = (Uint8 *)mi->bind_p;
		AG_INVFLAGS(*flags, mi->bind_flags);
		break;
	}
	case AG_MENU_INT16_FLAGS: {
		Uint16 *flags = (Uint16 *)mi->bind_p;
		AG_INVFLAGS(*flags, mi->bind_flags);
		break;
	}
	case AG_MENU_INT32_FLAGS: {
		Uint32 *flags = (Uint32 *)mi->bind_p;
		AG_INVFLAGS(*flags, mi->bind_flags);
		break;
	}
	case AG_MENU_NO_BINDING:
	default:
		break;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_SELF();
	AG_MenuItem *miRoot = mview->pitem, *mi;
	AG_Menu *m = mview->pmenu;
	int mx = AG_INT(2);
	int my = AG_INT(3);
	int y = mview->tPad;

	if (mx < 0 || mx >= WIDTH(mview) ||
	    my < 0 || my >= HEIGHT(mview)) {
		return;
	}
	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		AG_MenuUpdateItem(mi);

		y += m->itemh;
		if (my < y && mx >= 0 && mx <= WIDTH(mview)) {
			int fState = mi->stateFn ? mi->stateFn->fn.fnInt(mi->stateFn) :
			                           mi->state;
			if (!fState) {
				/* Nothing to do */
			} else if (mi->clickFn != NULL) {
				AG_MenuCollapseAll(m);
				mi->clickFn->fn.fnVoid(mi->clickFn);
			} else if (mi->bind_type != AG_MENU_NO_BINDING) {
				if (mi->bind_lock != NULL) {
					AG_MutexLock(mi->bind_lock);
				}
				SetItemBoolValue(mi);
				if (mi->bind_lock != NULL) {
					AG_MutexUnlock(mi->bind_lock);
				}
				AG_MenuCollapseAll(m);
			}
			AG_Redraw(mview);
			return;
		}
	}
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_SELF();

	/* XXX wasteful */
	if (mview->arrowRight == -1) {
		mview->arrowRight = AG_WidgetMapSurface(mview,
		    AG_SurfaceDup(agIconSmallArrowRight.s));
	}
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_SELF();
	AG_MenuItem *mi = mv->pitem, *miSub;
	int j;

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		for (j = 0; j < 2; j++) {
			if (miSub->lblView[j] != -1) {
				AG_WidgetUnmapSurface(mv, miSub->lblView[j]);
				miSub->lblView[j] = -1;
			}
		}
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_MenuView *mview = obj;

	WIDGET(mview)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                        AG_WIDGET_UNFOCUSED_BUTTONUP|
				AG_WIDGET_USE_TEXT;

	mview->pmenu = NULL;
	mview->pitem = NULL;
	mview->spIconLbl = 8;
	mview->spLblArrow = 16;
	mview->lPad = 8;
	mview->rPad = 8;
	mview->tPad = 4;
	mview->bPad = 4;
	mview->arrowRight = -1;
	AG_InitTimer(&mview->submenuTo, "submenu", 0);

	AG_SetEvent(mview, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(mview, "mouse-button-up", MouseButtonUp, NULL);
	AG_AddEvent(mview, "widget-shown", OnShow, NULL);
	AG_AddEvent(mview, "font-changed", OnFontChange, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_MenuView *mv = obj;
	AG_MenuItem *miRoot = mv->pitem, *mi;
	AG_Menu *m = mv->pmenu;
	AG_Font *font = WIDGET(mv)->font;
	AG_Rect r;
	int itemh = m->itemh, itemh_2 = (itemh >> 1);
	int fonth_2 = (font->height >> 1);

	r.x = 0;
	r.y = mv->tPad;
	r.w = WIDTH(mv);
	r.h = itemh;

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		AG_Color C;
		int x = mv->lPad;
		int boolState;
		int fState = mi->stateFn ? mi->stateFn->fn.fnInt(mi->stateFn) :
		                           mi->state;

		/* Update dynamic item if needed. */
		AG_MenuUpdateItem(mi);

		/* Indicate active item selection */
		if (mi == miRoot->sel_subitem && fState)
			AG_DrawRect(mv, r, WCOLOR_SEL(mv,0));

		/* Render the menu item's icon */
		if (mi->icon == -1 && mi->iconSrc != NULL) {
			mi->icon = AG_WidgetMapSurface(mv,
			    AG_SurfaceDup(mi->iconSrc));
		}
		if (mi->icon != -1) {
			AG_WidgetBlitSurface(mv, mi->icon,
			    x   + ((r.h >> 1) - (mi->iconSrc->w >> 1)),
			    r.y + ((r.h >> 1) - (mi->iconSrc->h >> 1)) + 1);

			/* Indicate boolean state */
			boolState = (mi->value != -1) ? mi->value :
			                                GetItemBoolValue(mi);
			if (boolState) {
				AG_Rect rFrame = AG_RECT(x, r.y+2, r.h, r.h-2);

				C = AG_ColorRGB(223,207,128);	/* XXX */
				AG_DrawFrame(mv, rFrame, 1, C);
				C.a = AG_OPAQUE/4;
				AG_DrawRectBlended(mv, rFrame, C, AG_ALPHA_SRC);
			}
		}

		/* Keep columns aligned if there are icons. */
		if (miRoot->flags & AG_MENU_ITEM_ICONS)
			x += itemh + mv->spIconLbl;

		if (mi->flags & AG_MENU_ITEM_SEPARATOR) {
			int x1 = mv->lPad;
			int x2 = WIDTH(mv) - mv->rPad - 1;
			AG_Color c[2];
	
			c[0] = AG_ColorAdd(WCOLOR(mv,0), agLowColor);
			c[1] = AG_ColorAdd(WCOLOR(mv,0), agHighColor);

			AG_DrawLineH(mv, x1, x2, (r.y + itemh_2 - 1), c[0]);
			AG_DrawLineH(mv, x1, x2, (r.y + itemh_2),     c[1]);
		} else {
			int lbl = fState ? mi->lblView[1] : mi->lblView[0];

			/* Render the menu item's text string */
			if (fState) {
				if (mi->lblView[1] == -1) {
					AG_TextColor(WCOLOR(mv,TEXT_COLOR));
					mi->lblView[1] =
					    (mi->text == NULL) ? -1 :
					    AG_WidgetMapSurface(mv,
					    AG_TextRender(mi->text));
				}
				lbl = mi->lblView[1];
			} else {
				if (mi->lblView[0] == -1) {
					AG_TextColor(WCOLOR_DIS(mv,TEXT_COLOR));
					mi->lblView[0] =
					    (mi->text == NULL) ? -1 :
					    AG_WidgetMapSurface(mv,
					    AG_TextRender(mi->text));
				}
				lbl = mi->lblView[0];
			}
			AG_WidgetBlitSurface(mv, lbl, x,
			    r.y + itemh_2 - fonth_2 + 1);
			x += WSURFACE(mv,lbl)->w;
		}

		/* Render the submenu arrow. */
		if (mi->nSubItems > 0) {
			x += mv->spLblArrow;
			AG_WidgetBlitSurface(mv, mv->arrowRight,
			    x,
			    r.y + itemh_2-(agIconSmallArrowRight.s->h >> 1)-1);
		}

		r.y += itemh;
	}
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *mi = mview->pitem, *item;
	AG_Menu *m = mview->pmenu;

	r->w = 0;
	r->h = mview->tPad + mview->bPad;
	
	TAILQ_FOREACH(item, &mi->subItems, items) {
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

		if (item->nSubItems > 0) {
			wReq += mview->spLblArrow + agIconSmallArrowRight.s->w;
		}
		if (wReq > r->w) {
			r->w = wReq;
		}
		r->h += m->itemh;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
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
