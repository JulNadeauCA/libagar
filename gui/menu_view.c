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

/* #define DEBUG_LINES */

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
MouseMotion(void *obj, int mx, int my, int dx, int dy)
{
	AG_MenuView *mv = obj;
	AG_MenuItem *mi = mv->pitem, *miSub;
	AG_Menu *m = mv->pmenu;
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
MouseButtonDown(void *obj, AG_MouseButton button, int x, int y)
{
	AG_MenuView *mv = obj;
	AG_Menu *m = mv->pmenu;
	AG_Driver *drv = WIDGET(mv)->drv;

	if (AGDRIVER_CLASS(drv)->setMouseAutoCapture != NULL)
		AGDRIVER_CLASS(drv)->setMouseAutoCapture(drv, 0);  /* Disable */

	if ((x < 0 || x >= WIDTH(mv) ||
	     y < 0 || y >= HEIGHT(mv)) &&
	    !AG_WidgetArea(m, x,y)) {
		if (AG_WidgetFindPoint("AG_Widget:AG_MenuView:*",
		    WIDGET(mv)->rView.x1 + x,
		    WIDGET(mv)->rView.y1 + y) == NULL)
			AG_MenuCollapseAll(m);
	}
}

static void
MouseButtonUp(void *obj, AG_MouseButton button, int mx, int my)
{
	AG_MenuView *mv = obj;
	AG_MenuItem *miRoot = mv->pitem, *mi;
	AG_Menu *m = mv->pmenu;
	AG_Driver *drv = WIDGET(mv)->drv;
	int y = WIDGET(mv)->paddingTop, itemh;

	if (AGDRIVER_CLASS(drv)->setMouseAutoCapture != NULL)
		AGDRIVER_CLASS(drv)->setMouseAutoCapture(drv, -1);   /* Reset */

	if (mx < 0 || mx >= WIDTH(mv) ||
	    my < 0 || my >= HEIGHT(mv))
		return;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	itemh = m->itemh;

	/* TODO: Scrolling */

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		int miState;

		if (mi->poll) {
			UpdateItem(m, mi);
		}
		if ((y += itemh) < my)
			continue;

		miState = mi->state;
		if (mi->stateFn != NULL) {
			AG_PostEventByPtr(m, mi->stateFn, "%p", &miState);
		}
		if (!miState) {
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
			AG_MenuBoolToggle(mi);
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
OnFontChange(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
	AG_Menu *m = mv->pmenu;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	m->itemh = WFONT(mv)->lineskip + m->tPadLbl + m->bPadLbl;
	AG_MenuInvalidateLabels(mv->pitem);

	AG_ObjectUnlock(m);
}

static void
OnDetach(AG_Event *_Nonnull event)
{
	AG_MenuView *mv = AG_MENUVIEW_SELF();
/*	AG_Menu *pmenu = mv->pmenu; */
	AG_MenuItem *mi;

/*	AG_ObjectLock(pmenu); */

	mi = mv->pitem;

	if (mi->view == mv)
		mi->view = NULL;

/*	AG_ObjectUnlock(pmenu); */
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

	AG_AddEvent(mv, "font-changed", OnFontChange, NULL);
	AG_AddEvent(mv, "detached", OnDetach, NULL);
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
	const int spacingHoriz = WIDGET(mv)->spacingHoriz;
	int indent, hasSubitemsWithChildren;
	
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

 	indent = (miRoot->flags & AG_MENU_ITEM_ICONS);

	hasSubitemsWithChildren = 0;
	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		if (mi->nSubItems > 0) {
			hasSubitemsWithChildren = 1;
			break;
		}
	}

	TAILQ_FOREACH(mi, &miRoot->subItems, items) {
		const int isEnabled = (mi->state);
		const int isSelected = (mi == miRoot->sel_subitem);
		const int yTextOffset = hItem_2 - fonth_2 + 1;
		int x = paddingLeft, *lbl;

		if (mi->flags & AG_MENU_ITEM_SEPARATOR) {      /* Separator */
			AG_Color c1 = WCOLOR(mv, FG_COLOR);
			AG_Color c2 = c1;
			const int x1 = paddingLeft;
			const int x2 = WIDTH(mv) - WIDGET(mv)->paddingRight - 1;
	
			AG_ColorDarken(&c1, 2);
			AG_ColorLighten(&c2, 2);
			AG_DrawLineH(mv, x1,x2, (r.y + hItem_2 - 1), &c1);
			AG_DrawLineH(mv, x1,x2, (r.y + hItem_2),     &c2);
#ifdef DEBUG_LINES
			AG_DrawLineH(mv, 0, WIDTH(mv), r.y, &WCOLOR(mv,LINE_COLOR));
#endif
			r.y += hItem;
			continue;
		}

		/* Call the boolean state evaluation function if one is set. */
		if (mi->stateFn != NULL)
			AG_PostEventByPtr(m, mi->stateFn, "%p", &isEnabled);

		/* Call the update routine if this is a dynamic item. */
		if (mi->poll)
			UpdateItem(m, mi);

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
			AG_Surface *S, *Stext, *Sb;

			S = AG_SurfaceStdRGB(r.w, r.h);
			AG_FillRect(S, NULL, isSelected ?
			    &WCOLOR(mv, SELECTION_COLOR) :
			    &WCOLOR(mv, BG_COLOR));

			/* TODO style */

			if (mi->bind_type != AG_MENU_NO_BINDING) {
				AG_Surface *Sb;
				const int boolValue = (mi->value != -1) ?
				    mi->value : AG_MenuBoolGet(mi);
				int y;

				if (boolValue) {
					Sb = AG_TextRender(AGSI_ALGUE AGSI_WHT
					                   AGSI_MENUBOOL_TRUE);
				} else {
					Sb = AG_TextRender(AGSI_ALGUE AGSI_WHT
					                   AGSI_MENUBOOL_FALSE);
				}
				y = ((r.h >> 1) - (Sb->h >> 1));
				if (y < 0) { y = 0; }
				AG_SurfaceBlit(Sb, NULL, S, x,y);
				x += Sb->w + spacingHoriz;
				AG_SurfaceFree(Sb);
			} else if (mi->icon) {
				int y = ((r.h >> 1) - (mi->icon->h >> 1));

				if (y < 0) { y = 0; }
				AG_SurfaceBlit(mi->icon, NULL, S, x,y);
				x += mi->icon->w + spacingHoriz;
			} else if (indent) {
				x += hItem + spacingHoriz;
			}

			AG_TextColor(isEnabled ?
			    &WCOLOR_DEFAULT(mv, TEXT_COLOR) :
			    &WCOLOR_DISABLED(mv, TEXT_COLOR));

			Stext = AG_TextRender(mi->text);
			AG_SurfaceBlit(Stext, NULL, S, x, yTextOffset);
			AG_SurfaceFree(Stext);

			if (mi->key_equiv != 0) {
				char buf[32];

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

				Sb = AG_TextRender(buf);
				if (Sb->w - r.w > 0) {
					int x = (r.w - Sb->w) - spacingHoriz;

					if (hasSubitemsWithChildren)
						x -= hItem;

					if (x > 0) {
						AG_SurfaceBlit(Sb, NULL, S,
						    x,
						    yTextOffset);
					}
				}
				AG_SurfaceFree(Sb);
			}

			if (mi->nSubItems > 0) {
				Sb = AG_TextRender(AGSI_ALGUE AGSI_WHT AGSI_MENU_EXPANDER);
				if (Sb->w < r.w) {
					AG_SurfaceBlit(Sb, NULL,
					    S, (r.w - Sb->w),
					    yTextOffset);
				}
				AG_SurfaceFree(Sb);
			}

			*lbl = AG_WidgetMapSurface(mv, S);

		}
		AG_WidgetBlitSurface(mv, *lbl, r.x, r.y);
#ifdef DEBUG_LINES
		AG_DrawLineH(mv, 0, WIDTH(mv), r.y, &WCOLOR(mv,LINE_COLOR));
#endif
		r.y += hItem;
	}

	AG_ObjectUnlock(m);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_MenuView *mv = obj;
	AG_MenuItem *miParent = mv->pitem, *mi;
	AG_Menu *m = mv->pmenu;
	const int hItem = m->itemh;
	const int paddingHoriz = WIDGET(mv)->paddingLeft +
	                         WIDGET(mv)->paddingRight;
	const int spacingHoriz = WIDGET(mv)->spacingHoriz;
	const int hasIcons = (miParent->flags & AG_MENU_ITEM_ICONS);

	r->w = 0;
	r->h = WIDGET(mv)->paddingTop + WIDGET(mv)->paddingBottom;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	
	TAILQ_FOREACH(mi, &miParent->subItems, items) {
		int wReq = paddingHoriz, wLbl;

		if (mi->poll)
			UpdateItem(m, mi);
		
		if (mi->bind_type != AG_MENU_NO_BINDING || hasIcons)
			wReq += hItem + spacingHoriz;
	
		AG_TextSize(mi->text, &wLbl, NULL);
		wReq += wLbl;

		if (mi->key_equiv != 0) {
			char buf[32];
			int wKey;

			Strlcpy(buf, " " AGSI_COURIER, sizeof(buf));
			if (mi->key_mod != 0) {
				char *kms;

				kms = AG_LookupKeyMod(mi->key_mod);
				Strlcat(buf, kms, sizeof(buf));
				Strlcat(buf, "-", sizeof(buf));
				free(kms);
			}
			Strlcat(buf, AG_LookupKeyName(mi->key_equiv), sizeof(buf));

			AG_TextSize(buf, &wKey, NULL);
			wReq += (spacingHoriz << 1) + wKey;
		}

		if (mi->nSubItems > 0) {
			wReq += hItem + spacingHoriz;
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
	NULL,			/* size_allocate */
	MouseButtonDown,
	MouseButtonUp,
	MouseMotion,
	NULL,			/* key_down */
	NULL,			/* key_up */
	NULL,			/* touch */
	NULL,			/* ctrl */
	NULL			/* joy */
};

#endif /* AG_WIDGETS */
