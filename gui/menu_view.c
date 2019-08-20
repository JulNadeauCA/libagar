/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#ifdef AG_WIDGETS

#include <agar/gui/menu.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>

static int  GetItemBoolValue(AG_MenuItem *_Nonnull);
static void SetItemBoolValue(AG_MenuItem *_Nonnull);

/* Refresh a dynamic item. */
static __inline__ void
UpdateItem(AG_Menu *_Nonnull m, AG_MenuItem *_Nonnull mi)
{
	AG_MenuInvalidateLabels(mi);
	AG_MenuFreeSubitems(mi);
	AG_PostEventByPtr(m, mi->poll, "%p", mi);
}

/* Selection has moved over the specified item. */
static void
SelectItem(AG_MenuItem *_Nonnull mi, AG_MenuItem *_Nullable subitem)
{
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->view != NULL) {
		AG_MenuCollapse(mi->sel_subitem);
	}
	mi->sel_subitem = subitem;

	if (subitem != NULL)
		AG_MenuExpand(mi->view, subitem, WIDTH(mi->view),
		              subitem->y + subitem->pmenu->itemh);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_MENUVIEW_SELF();
	AG_MenuItem *mi = mview->pitem, *miSub;
	AG_Menu *m = mview->pmenu;
	const int mx = AG_INT(1);
	const int my = AG_INT(2);
	const int w = WIDTH(mview);
	int y = mview->tPad, itemh;
	
	if (my < 0)
		return;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	itemh = m->itemh;
	
	if (mx < 0 || mx > w)
		goto no_select;

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		if (miSub->poll)
			UpdateItem(m, miSub); /* TODO regulate these updates */

		if ((y += itemh) <= my) {
			continue;
		}
		if ((mx > w && miSub->nSubItems == 0) ||
		    miSub->flags & AG_MENU_ITEM_SEPARATOR) {
			goto no_select;
		}
		if (mi->sel_subitem != miSub &&
		    (miSub->flags & AG_MENU_ITEM_NOSELECT) == 0) {
			SelectItem(mi, miSub);
		}
		AG_Redraw(mview);
		goto out;
	}
no_select:
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->nSubItems == 0) {
		AG_Redraw(mview);
		SelectItem(mi, NULL);
	}
out:
	AG_ObjectUnlock(m);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_MENUVIEW_SELF();
	AG_MenuItem *miRoot = mview->pitem;
	AG_Menu *m = mview->pmenu;
	const int mx = AG_INT(2);
	const int my = AG_INT(3);
	const int w = WIDTH(mview);
	const int h = HEIGHT(mview);

	if ((mx < 0 || mx >= w || my < 0 || my >= h) &&
	    !AG_WidgetArea(m, mx,my)) {
		if (AG_WidgetFindPoint("AG_Widget:AG_MenuView:*",
		    WIDGET(mview)->rView.x1 + mx,
		    WIDGET(mview)->rView.y1 + my) == NULL) {
			AG_MenuCollapse(miRoot);
			m->itemSel = NULL;
			m->selecting = 0;
		}
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_MENUVIEW_SELF();
	AG_MenuItem *miRoot = mview->pitem, *mi;
	AG_Menu *m = mview->pmenu;
	const int mx = AG_INT(2);
	const int my = AG_INT(3);
	const int w = WIDTH(mview);
	const int h = HEIGHT(mview);
	int y = mview->tPad, itemh;
#if 1
	if (mx < 0 || mx >= w || my < 0 || my >= h) {
		Debug(mview, "MouseButtonUp: Out of bounds\n");
		return;
	}
#endif
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	itemh = m->itemh;

	/* TODO: Scrolling */

	Debug(mview, "click @%d,%d (in %dx%d, itemh = %d)\n", mx,my, w,h, itemh);

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		int mi_state;

		if (mi->poll) {
			Debug(mview, "Updating [%s] (lbl=[%d,%d])\n", mi->text,
			    mi->lblView[0],
			    mi->lblView[1]);
			UpdateItem(m, mi);
		}
		if ((y += itemh) < my) {
			continue;
		}
		Debug(mview, "Item: [%s]\n", mi->text);
		Debug(mview, "[%s]: Processing (state=%d)\n", mi->text, mi->state);

		mi_state = mi->state;
		if (mi->stateFn != NULL) {
			AG_PostEventByPtr(m, mi->stateFn, "%p", &mi_state);
		}
		if (!mi_state) {
			/* Nothing to do */
			Debug(mview, "[%s]: Inactive item\n", mi->text);
		} else if (mi->clickFn != NULL) {
			AG_MenuCollapseAll(m);
			if (mi->clickFn->fn != NULL) {
				Debug(mview, "[%s]: Calling %p\n", mi->text,
				    mi->clickFn);
				mi->clickFn->fn(mi->clickFn);
			} else {
				Debug(mview, "[%s]: No clickFn\n", mi->text);
			}
		} else if (mi->bind_type != AG_MENU_NO_BINDING) {
#ifdef AG_THREADS
			if (mi->bind_lock) { AG_MutexLock(mi->bind_lock); }
#endif
			Debug(mview, "[%s]: Setting bool\n", mi->text);
			SetItemBoolValue(mi);
#ifdef AG_THREADS
			if (mi->bind_lock) { AG_MutexUnlock(mi->bind_lock); }
#endif
			AG_MenuCollapseAll(m);
		}
		AG_Redraw(mview);
		
		if (y > my) {
			Debug(mview, "[%s]: Break\n", mi->text);
			break;
		}
	}
	AG_ObjectUnlock(m);
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
OnShow(AG_Event *_Nonnull event)
{
	AG_MenuView *mview = AG_MENUVIEW_SELF();

	/* XXX wasteful */
	if (mview->arrowRight == -1) {
		mview->arrowRight = AG_WidgetMapSurface(mview,
		    AG_SurfaceDup(agIconSmallArrowRight.s));
	}
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_MenuItem *mi = mv->pitem, *miSub;
	AG_Menu *m = mv->pmenu;
	int j;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		for (j = 0; j < 2; j++) {
			if (miSub->lblView[j] != -1) {
				AG_WidgetUnmapSurface(mv, miSub->lblView[j]);
				miSub->lblView[j] = -1;
			}
		}
	}

	AG_ObjectUnlock(m);
}

static void
Init(void *_Nonnull obj)
{
	AG_MenuView *mview = obj;

	WIDGET(mview)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
	                        AG_WIDGET_UNFOCUSED_BUTTONDOWN |
	                        AG_WIDGET_UNFOCUSED_BUTTONUP |
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

	AG_SetEvent(mview, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(mview, "mouse-button-down", MouseButtonDown, NULL);
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
	const AG_Font *font = WFONT(mv);
	AG_Rect r;
	const int itemh = m->itemh;
	const int itemh_2 = (itemh >> 1);
	const int fonth_2 = (font->height >> 1);
	
	r.x = 0;
	r.y = mv->tPad;
	r.w = WIDTH(mv);
	r.h = itemh;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		AG_Color c;
		const AG_Surface *Sicon = mi->iconSrc;
		const int mi_state = mi->state;
		int x = mv->lPad;

		if (mi->stateFn != NULL)
			AG_PostEventByPtr(m, mi->stateFn, "%p", &mi_state);

		if (mi->poll)
			UpdateItem(m, mi);

		if (mi == miRoot->sel_subitem && mi_state)    /* Is selected */
			AG_DrawRect(mv, &r, &WCOLOR_SEL(mv,0));

		if (mi->icon == -1 && Sicon) {
			mi->icon = AG_WidgetMapSurface(mv, AG_SurfaceDup(Sicon));
		}
		if (mi->icon != -1) {
			int bv;

			AG_WidgetBlitSurface(mv, mi->icon,
			    x   + ((r.h >> 1) - (Sicon->w >> 1)),
			    r.y + ((r.h >> 1) - (Sicon->h >> 1)) + 1);

			bv = (mi->value != -1) ? mi->value : GetItemBoolValue(mi);
			if (bv) {
				AG_Rect rFrame;

				rFrame.x = x;
				rFrame.y = r.y + 2;
				rFrame.w = r.h;
				rFrame.h = r.h - 2;

				AG_ColorRGB_8(&c, 223,207,128);	/* XXX */
				AG_DrawFrame(mv, &rFrame, 1, &c);
				c.a = AG_OPAQUE/4;
				AG_DrawRectBlended(mv, &rFrame, &c,
				    AG_ALPHA_SRC,
				    AG_ALPHA_ONE_MINUS_SRC);
			}
		}

		/* Keep columns aligned if there are icons. */
		if (miRoot->flags & AG_MENU_ITEM_ICONS)
			x += itemh + mv->spIconLbl;

		if (mi->flags & AG_MENU_ITEM_SEPARATOR) {
			AG_Color c[2];
			int x1 = mv->lPad;
			int x2 = WIDTH(mv) - mv->rPad - 1;
			
			AG_ColorAdd(&c[0], &WCOLOR(mv,0), &agLowColor);
			AG_ColorAdd(&c[1], &WCOLOR(mv,0), &agHighColor);
			AG_DrawLineH(mv, x1, x2, (r.y + itemh_2 - 1), &c[0]);
			AG_DrawLineH(mv, x1, x2, (r.y + itemh_2),     &c[1]);
		} else {
			int lbl;

			/* Render the menu item's text string */
			if (mi_state) {
				if (mi->lblView[1] == -1) {
					AG_TextColor(&WCOLOR(mv,TEXT_COLOR));
					mi->lblView[1] = (mi->text==NULL) ? -1 :
					    AG_WidgetMapSurface(mv,
					    AG_TextRender(mi->text));
				}
				lbl = mi->lblView[1];
			} else {
				if (mi->lblView[0] == -1) {
					AG_TextColor(&WCOLOR_DIS(mv,TEXT_COLOR));
					mi->lblView[0] =
					    (mi->text == NULL) ? -1 :
					    AG_WidgetMapSurface(mv,
					    AG_TextRender(mi->text));
				}
				lbl = mi->lblView[0];
			}
			AG_WidgetBlitSurface(mv, lbl, x, r.y+itemh_2-fonth_2+1);
			x += WSURFACE(mv,lbl)->w;
		}

		/* Render the submenu arrow. */
		/* TODO use a vector icon */
		if (mi->nSubItems > 0) {
			x += mv->spLblArrow;
			AG_WidgetBlitSurface(mv, mv->arrowRight,
			    x,
			    r.y + itemh_2-(agIconSmallArrowRight.s->h >> 1)-1);
		}

		r.y += itemh;
	}
	
	AG_ObjectUnlock(m);
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
	return (mi->flags & AG_MENU_ITEM_INVERTED) ? !val : val;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_MenuView *mview = obj;
	AG_MenuItem *mi = mview->pitem, *item;
	AG_Menu *m = mview->pmenu;
	const int itemh = m->itemh;

	r->w = 0;
	r->h = mview->tPad + mview->bPad;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	
	TAILQ_FOREACH(item, &mi->subItems, items) {
		int wReq = mview->lPad + mview->rPad;
		int wLbl;

		if (item->poll)
			UpdateItem(m, item);
		
		if (item->icon != -1) {
			wReq += item->iconSrc->w;
		}
		if (mi->flags & AG_MENU_ITEM_ICONS)
			wReq += itemh + mview->spIconLbl;
	
		AG_TextSize(item->text, &wLbl, NULL);
		wReq += wLbl;

		if (item->nSubItems > 0) {
			wReq += mview->spLblArrow + agIconSmallArrowRight.s->w;
		}
		if (wReq > r->w) {
			r->w = wReq;
		}
		r->h += itemh;
	}
	
	AG_ObjectUnlock(m);
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
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
