/*
 * Copyright (c) 2004-2022 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Menu display widget (spawned by AG_Menu(3) when a menu is expanded).
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
SelectItem(AG_MenuItem *_Nonnull mi, AG_MenuItem *_Nullable miSub)
{
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->view != NULL) {
		AG_MenuCollapse(mi->sel_subitem);
	}
	mi->sel_subitem = miSub;

	if (miSub != NULL && mi->view != NULL)
		AG_MenuExpand(mi->view, miSub, WIDTH(mi->view),
		    miSub->y + miSub->pmenu->itemh);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_MenuItem *mi = mv->pitem, *miSub;
	AG_Menu *m = mv->pmenu;
	const int mx = AG_INT(1);
	const int my = AG_INT(2);
	const int w = WIDTH(mv);
	int y = WIDGET(mv)->paddingTop, itemh;
	
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
		AG_Redraw(mv);
		goto out;
	}
no_select:
	if (mi->sel_subitem != NULL &&
	    mi->sel_subitem->nSubItems == 0) {
		AG_Redraw(mv);
		SelectItem(mi, NULL);
	}
out:
	AG_ObjectUnlock(m);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_Menu *m = mv->pmenu;
	const int mx = AG_INT(2);
	const int my = AG_INT(3);

	if ((mx < 0 || mx >= WIDTH(mv) ||
	     my < 0 || my >= HEIGHT(mv)) &&
	    !AG_WidgetArea(m, mx,my)) {
		if (AG_WidgetFindPoint("AG_Widget:AG_MenuView:*",
		    WIDGET(mv)->rView.x1 + mx,
		    WIDGET(mv)->rView.y1 + my) == NULL)
			AG_MenuCollapseAll(m);
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_MenuItem *miRoot = mv->pitem, *mi;
	AG_Menu *m = mv->pmenu;
	const int mx = AG_INT(2);
	const int my = AG_INT(3);
	int y = WIDGET(mv)->paddingTop, itemh;

	if (mx < 0 || mx >= WIDTH(mv) ||
	    my < 0 || my >= HEIGHT(mv))
		return;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	itemh = m->itemh;

	/* TODO: Scrolling */

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		int mi_state;

		if (mi->poll) {
			UpdateItem(m, mi);
		}
		if ((y += itemh) < my)
			continue;

		mi_state = mi->state;
		if (mi->stateFn != NULL) {
			AG_PostEventByPtr(m, mi->stateFn, "%p", &mi_state);
		}
		if (!mi_state) {
			/* Nothing to do */
		} else if (mi->clickFn != NULL) {
			AG_MenuCollapseAll(m);
			if (mi->clickFn->fn != NULL) {
				mi->clickFn->fn(mi->clickFn);
			}
		} else if (mi->bind_type != AG_MENU_NO_BINDING) {
#ifdef AG_THREADS
			if (mi->bind_lock) { AG_MutexLock(mi->bind_lock); }
#endif
			SetItemBoolValue(mi);
#ifdef AG_THREADS
			if (mi->bind_lock) { AG_MutexUnlock(mi->bind_lock); }
#endif
			AG_MenuCollapseAll(m);
		}
		AG_Redraw(mv);
		
		if (y > my)
			break;
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
OnFontChange(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_Menu *m = mv->pmenu;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	m->itemh = WFONT(mv)->lineskip + m->tPadLbl + m->bPadLbl;
	AG_MenuInvalidateLabels(mv->pitem);

	AG_ObjectUnlock(m);
#if 0
	if ((win = WIDGET(mv)->window) != NULL) {        /* Resize */
		AG_WindowSetGeometry(win,
		    WIDGET(win)->x,
		    WIDGET(win)->y, -1,-1);
	}
#endif
}

static void
Init(void *_Nonnull obj)
{
	AG_MenuView *mv = obj;

	WIDGET(mv)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
	                     AG_WIDGET_UNFOCUSED_BUTTONDOWN |
	                     AG_WIDGET_UNFOCUSED_BUTTONUP |
	                     AG_WIDGET_USE_TEXT;

	mv->pmenu = NULL;
	mv->pitem = NULL;
	mv->spLblArrow = 16;
	mv->arrowRight = -1;

	AG_SetEvent(mv, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(mv, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(mv, "mouse-button-up", MouseButtonUp, NULL);
	AG_AddEvent(mv, "font-changed", OnFontChange, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_MenuView *mv = obj;
	AG_MenuItem *miRoot = mv->pitem, *mi;
	AG_Menu *m = mv->pmenu;
	const AG_Font *font = WFONT(mv);
	AG_Rect r;
	const int hItem = m->itemh;
	const int hItem_2 = (hItem >> 1);
	const int fonth_2 = (font->lineskip >> 1);
	const int paddingLeft = WIDGET(mv)->paddingLeft;
	const int menuHasIcons = (miRoot->flags & AG_MENU_ITEM_ICONS);
	const int spacingHoriz = WIDGET(mv)->spacingHoriz;

	if (agDriverSw) {
		r = WIDGET(mv)->r;
		r.x++; r.w--;
		r.h--;
		AG_DrawRectOutline(mv, &r, &WCOLOR(mv,LINE_COLOR));
		r.y++;
		AG_DrawRect(mv, &r, &WCOLOR(mv,BG_COLOR));
	} else {
		r = WIDGET(mv)->r;
		r.x++;
		r.w--;
		r.h--;
		AG_DrawRectOutline(mv, &r, &WCOLOR(mv,LINE_COLOR));
	}

	r.x = 1;
	r.y = WIDGET(mv)->paddingTop;
	r.w = WIDTH(mv) - r.x - 1;
	r.h = hItem;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

/*	AG_PushBlendingMode(mv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC); */

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		const int isEnabled = (mi->state);
		const int isSelected = (mi == miRoot->sel_subitem);
		const int yTextOffset = hItem_2 - fonth_2 + 1;
		int x = paddingLeft, boolVal, *lbl;

		/* Call the boolean state evaluation function if one is set. */
		if (mi->stateFn != NULL)
			AG_PostEventByPtr(m, mi->stateFn, "%p", &isEnabled);

		/* Call the update routine if this is a dynamic item. */
		if (mi->poll)
			UpdateItem(m, mi);

		boolVal = (mi->value != -1) ? mi->value : GetItemBoolValue(mi);
		if (boolVal) {
			AG_Rect rFrame;
			AG_Color c;

			/* TODO style */

			rFrame.x = x;
			rFrame.y = r.y + 2;
			rFrame.w = r.h - 4;
			rFrame.h = r.h - 4;
			AG_DrawFrameRaised(mv, &rFrame);
			c = WCOLOR(mv, SELECTION_COLOR);
			c.a = AG_OPAQUE/4;
			AG_DrawRectBlended(mv, &rFrame, &c,
			    AG_ALPHA_SRC,
			    AG_ALPHA_ONE_MINUS_SRC);
		}

		if (mi->flags & AG_MENU_ITEM_SEPARATOR) {      /* Separator */
			AG_Color c1 = WCOLOR(mv, FG_COLOR);
			AG_Color c2 = c1;
			const int x1 = paddingLeft;
			const int x2 = WIDTH(mv) - WIDGET(mv)->paddingRight - 1;
	
			AG_ColorDarken(&c1, 2);
			AG_ColorLighten(&c2, 2);
			AG_DrawLineH(mv, x1,x2, (r.y + hItem_2 - 1), &c1);
			AG_DrawLineH(mv, x1,x2, (r.y + hItem_2),     &c2);

			r.y += hItem;
			continue;
		}

		if (isEnabled) {
			if (isSelected) {
				lbl = &mi->lblView[2];
			} else {
				lbl = &mi->lblView[1];
			}
		} else {
			lbl = &mi->lblView[0];
		}

		if (*lbl == -1) {
			AG_Surface *S, *Stext;

			S = AG_SurfaceStdRGBA(r.w, r.h);
			AG_FillRect(S, NULL, isSelected ?
			    &WCOLOR(mv, SELECTION_COLOR) :
			    &WCOLOR(mv, BG_COLOR));

			/* TODO style */

			if (mi->iconSrc) {
				AG_SurfaceBlit(mi->iconSrc, NULL, S, 
				    ((r.h >> 1) - (mi->iconSrc->w >> 1)),
				    ((r.h >> 1) - (mi->iconSrc->h >> 1)));
			}
			if (menuHasIcons)
				x += hItem + spacingHoriz;

			AG_TextColor(isEnabled ?
			    &WCOLOR(mv, TEXT_COLOR) :
			    &WCOLOR_DISABLED(mv, TEXT_COLOR));

			Stext = AG_TextRender(mi->text);
			AG_SurfaceBlit(Stext, NULL, S, x, yTextOffset);
			AG_SurfaceFree(Stext);

			if (mi->key_equiv != 0) {
				char buf[32];
				AG_Surface *Sbuf;

				Strlcpy(buf, " " AGSI_COURIER, sizeof(buf));

				if (mi->key_mod != 0) {
					char *kms;

					kms = AG_LookupKeyMod(mi->key_mod);
					Strlcat(buf, kms, sizeof(buf));
					Strlcat(buf, "-", sizeof(buf));
					free(kms);
				}

				Strlcat(buf, AG_LookupKeyName(mi->key_equiv),
				    sizeof(buf));

				Sbuf = AG_TextRender(buf);
				if (Sbuf->w - r.w > 0) {
					AG_SurfaceBlit(Sbuf, NULL, S,
					    (r.w - Sbuf->w - spacingHoriz),
					    yTextOffset);
				}
				AG_SurfaceFree(Sbuf);
			}

			if (mi->nSubItems > 0) {
				AG_Surface *Sarrow = agIconSmallArrowRight.s;

				if (Sarrow->w < r.w) {
					AG_SurfaceBlit(Sarrow, NULL,
					    S, (r.w - Sarrow->w),
					    r.y + hItem_2 - (Sarrow->h >> 1) - 1);
				}
			}

			*lbl = AG_WidgetMapSurface(mv, S);
		}
		AG_WidgetBlitSurface(mv, *lbl, r.x, r.y);
		r.y += hItem;
	}

/*	AG_PopBlendingMode(mv); */

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
	AG_MenuView *mv = obj;
	AG_MenuItem *mi = mv->pitem, *miSub;
	AG_Menu *m = mv->pmenu;
	const int hItem = m->itemh;
	const int paddingHoriz = WIDGET(mv)->paddingLeft +
	                         WIDGET(mv)->paddingRight;

	r->w = 0;
	r->h = WIDGET(mv)->paddingTop + WIDGET(mv)->paddingBottom;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	
	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		int wReq = paddingHoriz, wLbl;

		if (miSub->poll)
			UpdateItem(m, miSub);
		
		if (mi->flags & AG_MENU_ITEM_ICONS) {
			if (miSub->icon != -1) {
				wReq += MAX(hItem, miSub->iconSrc->w);
			} else {
				wReq += hItem;
			}
			wReq += WIDGET(mv)->spacingHoriz;
		}
	
		AG_TextSize(miSub->text, &wLbl, NULL);
		wReq += wLbl;

		if (miSub->key_equiv != 0) {
			char buf[32];
			int wKey;

			Strlcpy(buf, " " AGSI_COURIER, sizeof(buf));
			if (miSub->key_mod != 0) {
				char *kms;

				kms = AG_LookupKeyMod(miSub->key_mod);
				Strlcat(buf, kms, sizeof(buf));
				Strlcat(buf, "-", sizeof(buf));
				free(kms);
			}
			Strlcat(buf, AG_LookupKeyName(miSub->key_equiv), sizeof(buf));

			AG_TextSize(buf, &wKey, NULL);
			wReq += WIDGET(mv)->spacingHoriz + wKey;
		}

		if (miSub->nSubItems > 0) {
			wReq += WIDGET(mv)->spacingHoriz + mv->spLblArrow +
			        agIconSmallArrowRight.s->w;
		}
		if (wReq > r->w) {
			r->w = wReq;
		}
		r->h += hItem;
	}

	AG_ObjectUnlock(m);
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
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */
