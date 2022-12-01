/*
 * Copyright (c) 2002-2022 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Tree/List View widget. It shows a scrollable tree (or list) of clickable
 * and selectable text items. It provides a polling mode with asset-recycling
 * for lists which must be cleared and repopulated frequently.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/tlist.h>
#include <agar/gui/primitive.h>

#include <string.h>
#include <stdarg.h>

#ifndef AG_TLIST_PADDING
#define AG_TLIST_PADDING 2	/* Label padding (pixels). XXX */
#endif

#ifndef AG_TLIST_EXP_LEVELS_INIT
#define AG_TLIST_EXP_LEVELS_INIT 8  /* Initial tree-expansion state buffer size */
#endif

static void SelectRange(AG_Tlist *_Nonnull, int);
static void DrawExpColl(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull, int,int);
static Uint32 PollRefreshTimeout(AG_Timer *_Nonnull, AG_Event *_Nonnull);

AG_Tlist *
AG_TlistNew(void *parent, Uint flags)
{
	AG_Tlist *tl;

	tl = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(tl, &agTlistClass);

	if (flags & AG_TLIST_HFILL) { WIDGET(tl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_TLIST_VFILL) { WIDGET(tl)->flags |= AG_WIDGET_VFILL; }
	tl->flags |= flags;

	AG_ObjectAttach(parent, tl);
	return (tl);
}

AG_Tlist *
AG_TlistNewPolled(void *parent, Uint flags, AG_EventFn fn, const char *fmt, ...)
{
	AG_Tlist *tl;
	AG_Event *ev;

	tl = AG_TlistNew(parent, flags);

	AG_ObjectLock(tl);
	tl->flags |= AG_TLIST_POLL;
	ev = AG_SetEvent(tl, "tlist-poll", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(tl);

	AG_RedrawOnTick(tl, 1000);
	return (tl);
}

AG_Tlist *
AG_TlistNewPolledMs(void *parent, Uint flags, int ms, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Tlist *tl;
	AG_Event *ev;

	tl = AG_TlistNew(parent, flags);

	AG_ObjectLock(tl);
	tl->flags |= AG_TLIST_POLL;
	ev = AG_SetEvent(tl, "tlist-poll", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	tl->pollDelay = ms;
	AG_ObjectUnlock(tl);

	AG_RedrawOnTick(tl, ms);
	return (tl);
}

/* Set the refresh rate for Polled mode in milliseconds (-1 = disable) */
void
AG_TlistSetRefresh(AG_Tlist *tl, int ms)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if (ms == -1) {
		AG_DelTimer(tl, &tl->refreshTo);
	} else {
		AG_AddTimer(tl, &tl->refreshTo, ms, PollRefreshTimeout, NULL);
	}
	tl->pollDelay = ms;

	AG_RedrawOnTick(tl, ms);
	AG_ObjectUnlock(tl);
}

/* In AG_TLIST_POLL mode, invoke `tlist-poll' if refresh timer has expired. */
static __inline__ void
UpdatePolled(AG_Tlist *_Nonnull tl)
{
	if ((tl->flags & AG_TLIST_POLL) &&
	    (tl->flags & AG_TLIST_REFRESH)) {
		tl->flags &= ~(AG_TLIST_REFRESH);
		AG_PostEvent(tl, "tlist-poll", NULL);
	}
}

/* Return 1 if at least one selected item is visible */
static int
SelectionVisible(AG_Tlist *_Nonnull tl)
{
	AG_TlistItem *it;
	int y=0, i=0, rOffs, yLast, item_h;

	UpdatePolled(tl);

	item_h = tl->item_h;
	yLast = HEIGHT(tl)-item_h;
	rOffs = tl->rOffs;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (i++ < rOffs)
			continue;
		if (y > yLast)
			break;
		if (it->selected)
			return (1);

		y += item_h;
	}
	return (0);
}

/* Scroll to the first visible item. */
static void
ScrollToSelection(AG_Tlist *_Nonnull tl)
{
	AG_TlistItem *it;
	int m=0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected) {
			m++;
			continue;
		}
		tl->rOffs = (tl->rOffs > m) ? m : MAX(0, m - tl->nVisible+1);
		AG_Redraw(tl);
		return;
	}
}

static void
SelectItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_Variable *selectedb;
	void **sel_ptr;

	selectedb = AG_GetVariable(tl, "selected", (void *)&sel_ptr);
	*sel_ptr = it->p1;
	if (!it->selected) {
		it->selected = 1;
		if (tl->changedEv) {
			AG_PostEventByPtr(tl, tl->changedEv, "%p,%i", it, 1);
		}
		AG_PostEvent(tl, "tlist-changed", "%p,%i", it, 1);
	}
	if ((tl->flags & AG_TLIST_NO_SELECTED) == 0) {
		AG_PostEvent(tl, "tlist-selected", "%p", it);
	}
	AG_UnlockVariable(selectedb);
	AG_Redraw(tl);
}

static void
DeselectItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_Variable *selectedb;
	void **sel_ptr;

	selectedb = AG_GetVariable(tl, "selected", (void *)&sel_ptr);
	*sel_ptr = NULL;
	if (it->selected) {
		it->selected = 0;
		if (tl->changedEv) {
			AG_PostEventByPtr(tl, tl->changedEv, "%p,%i", it, 0);
		}
		AG_PostEvent(tl, "tlist-changed", "%p,%i", it, 0);
	}
	AG_UnlockVariable(selectedb);
	AG_Redraw(tl);
}

static void
DecrementSelection(AG_Tlist *_Nonnull tl, int inc)
{
	AG_TlistItem *it, *itPrev;
	int i;

	for (i = 0; i < inc; i++) {
		TAILQ_FOREACH(it, &tl->items, items) {
			if (!it->selected) {
				continue;
			}
prev_item:
			itPrev = TAILQ_PREV(it, ag_tlist_itemq, items);
			if (itPrev) {
				if (itPrev->flags & AG_TLIST_NO_SELECT) {
					if (itPrev != TAILQ_FIRST(&tl->items)) {
						DeselectItem(tl, it);
						it = itPrev;
						goto prev_item;
					} else {
						break;
					}
				}
				DeselectItem(tl, it);
				SelectItem(tl, itPrev);
			}
			break;
		}
		if (it == NULL && (it = TAILQ_FIRST(&tl->items)) != NULL)
			SelectItem(tl, it);
	}
	if (!SelectionVisible(tl))
		ScrollToSelection(tl);
}

static void
IncrementSelection(AG_Tlist *_Nonnull tl, int inc)
{
	AG_TlistItem *it, *itNext;
	int i;

	for (i = 0; i < inc; i++) {
		TAILQ_FOREACH(it, &tl->items, items) {
			if (!it->selected)
				continue;
next_item:
			itNext = TAILQ_NEXT(it, items);
			if (itNext) {
				if (itNext->flags & AG_TLIST_NO_SELECT) {
					DeselectItem(tl, it);
					it = itNext;
					goto next_item;
				}
				DeselectItem(tl, it);
				SelectItem(tl, itNext);
			}
			break;
		}
		if (it == NULL && (it = TAILQ_FIRST(&tl->items)) != NULL)
			SelectItem(tl, it);
	}
	if (!SelectionVisible(tl))
		ScrollToSelection(tl);
}

static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	tl->dblClicked = NULL;
	return (0);
}

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	if (tl->flags & AG_TLIST_POLL) {
		tl->flags &= ~(AG_TLIST_REFRESH);
		AG_DelTimer(tl, &tl->refreshTo);
	}
	AG_DelTimer(tl, &tl->moveTo);
	AG_DelTimer(tl, &tl->dblClickTo);
}

static void
OnLostFocus(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	AG_DelTimer(tl, &tl->moveTo);
	AG_DelTimer(tl, &tl->dblClickTo);
}

/* Timer for updates in AG_TLIST_POLL mode. */
static Uint32
PollRefreshTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	tl->flags |= AG_TLIST_REFRESH;
	AG_Redraw(tl);
	return (to->ival);
}

static __inline__ void
InvalidateLabels(AG_Tlist *tl, AG_TlistItem *it)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (it->label[i] == -1) {
			continue;
		}
		AG_WidgetUnmapSurface(tl, it->label[i]);
		it->label[i] = -1;
	}
}

static void
FontChanged(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_TlistItem *it;
	int i;

	TAILQ_FOREACH(it, &tl->items, items) {
		InvalidateLabels(tl, it);
	}
	if ((tl->flags & AG_TLIST_FIXED_HEIGHT) == 0) {
		AG_TlistSetItemHeight(tl, WFONT(tl)->lineskip + AG_TLIST_PADDING*2);
		AG_TlistSetIconWidth(tl, tl->item_h + 1);
	}
	for (i = 0; i < AG_WIDGET_NSTATES; i++) {
		AG_ColorInterpolate(&tl->cBgLine[i],
		    &WIDGET(tl)->pal.c[i][AG_BG_COLOR],
		    &WIDGET(tl)->pal.c[i][AG_LINE_COLOR],
		    1,3);
	}
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	if (tl->flags & AG_TLIST_POLL) {
		tl->flags |= AG_TLIST_REFRESH;
		AG_AddTimer(tl, &tl->refreshTo, tl->pollDelay,
		    PollRefreshTimeout, NULL);
	}
}

/* Timer for moving keyboard selection. */
static Uint32
MoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const int incr = AG_INT(1);

	if (incr < 0) {
		DecrementSelection(tl, -incr);
	} else {
		IncrementSelection(tl, incr);
	}
	return (agKbdRepeat);
}

void
AG_TlistSizeHint(AG_Tlist *tl, const char *text, int nItems)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	AG_TextSize(text, &tl->wHint, NULL);
	tl->hHint = (tl->item_h + 2)*nItems;

	AG_ObjectUnlock(tl);
}

void
AG_TlistSizeHintPixels(AG_Tlist *tl, int w, int nItems)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->wHint = w;
	tl->hHint = (tl->item_h + 2)*nItems;

	AG_ObjectUnlock(tl);
}

/*
 * Set the default size hint to accomodate the largest text label in
 * the current list of items, and the given number of items.
 */
void
AG_TlistSizeHintLargest(AG_Tlist *tl, int nItems)
{
	AG_TlistItem *it;
	int w, wHint=0;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	UpdatePolled(tl);
	wHint = 0;
	AG_TLIST_FOREACH(it, tl) {
		AG_TextSize(it->text, &w, NULL);
		if (w > wHint) { wHint = w; }
	}
	tl->wHint = wHint;
	tl->hHint = (tl->item_h + 2) * nItems;

	AG_ObjectUnlock(tl);
}

static __inline__ void
FreeItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (it->label[i] != -1)
			AG_WidgetUnmapSurface(tl, it->label[i]);
	}
	if (it->iconsrc) {
		AG_SurfaceFree(it->iconsrc);
	}
	if (it->color)
		free(it->color);
#if 0
	if (it->font)
		AG_UnusedFont(it->font);
#endif
	free(it);
}

static void
Destroy(void *_Nonnull p)
{
	AG_Tlist *tl = p;
	AG_TlistItem *it, *nit;
	AG_TlistPopup *tp, *ntp;

	for (it = TAILQ_FIRST(&tl->selitems);
	     it != TAILQ_END(&tl->selitems);
	     it = nit) {
		nit = TAILQ_NEXT(it, selitems);
		FreeItem(tl, it);
	}
	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		FreeItem(tl, it);
	}
	for (tp = TAILQ_FIRST(&tl->popups);
	     tp != TAILQ_END(&tl->popups);
	     tp = ntp) {
		ntp = TAILQ_NEXT(tp, popups);
		AG_ObjectDestroy(tp->menu);
		free(tp);
	}
	free(tl->expLevels);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Tlist *tl = obj;
	AG_SizeReq rBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);

	r->w = WIDGET(tl)->paddingLeft +
	       tl->icon_w + WIDGET(tl)->spacingHoriz + tl->wHint + rBar.w +
	       WIDGET(tl)->paddingRight;

	r->h = WIDGET(tl)->paddingTop + tl->hHint +
	       WIDGET(tl)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Tlist *tl = obj;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);
	if (a->w < (rBar.w << 1)) {
		rBar.w = MAX(0, (a->w >> 1));
	}
	aBar.w = rBar.w;
	aBar.h = a->h;
	aBar.x = a->w - rBar.w;
	aBar.y = WIDGET(tl)->paddingTop;
	AG_WidgetSizeAlloc(tl->sbar, &aBar);

	tl->r.x = 0;
	tl->r.y = 0;
	tl->r.w = a->w - aBar.w + 1;
	tl->r.h = a->h - 1;

	tl->nVisible = a->h/tl->item_h;              /* Vertical scrollbar */
	if (tl->rOffs + tl->nVisible >= tl->nItems) {
		tl->rOffs = MAX(0, tl->nItems - tl->nVisible);
	}
	return (0);
}

/*
 * Tag a tree level as Expanded (y > 0) or Collapsed (y = 0). Recording the
 * y coordinate of the expansion point is necessary for backtracking.
 */
static __inline__ void
SetExpansionLevel(AG_Tlist *_Nonnull tl, int level, int y)
{
	if (level > tl->nExpLevels) {
		tl->nExpLevels = level;
		tl->expLevels = Realloc(tl->expLevels, level * sizeof(int));
		tl->expLevels[tl->nExpLevels++] = y;
	} else {
		tl->expLevels[level] = y;
	}
}

static void
Draw(void *_Nonnull obj)
{
	AG_Tlist *tl = obj;
	AG_TlistItem *it;
	const int h = HEIGHT(tl);
	const int paddingLeft  = WIDGET(tl)->paddingLeft;
	const int spacingHoriz = WIDGET(tl)->spacingHoriz;
	const int hItem = tl->item_h;
	const int hItem_2 = (hItem >> 1);
	const int rOffs = tl->rOffs;
	const int disabled = (WIDGET(tl)->flags & AG_WIDGET_DISABLED);
	const AG_Color *cLine   = &WCOLOR(tl,LINE_COLOR);
	const AG_Color *cBg     = &WCOLOR(tl,BG_COLOR);
	const AG_Color *cBgLine = &tl->cBgLine[WIDGET(tl)->state];
	AG_Rect r = tl->r;
	int y, i=0, j, selSeen=0, selPos=1, yLast;

	TAILQ_FOREACH(it, &tl->items, items)
		InvalidateLabels(tl, it);

	UpdatePolled(tl);

	memset(tl->expLevels, 0, tl->nExpLevels * sizeof(int));

	AG_DrawBoxSunk(tl, &r, cBg);                     /* Background area */

	AG_WidgetDraw(tl->sbar);

	r.w--;
	r.h--;
	AG_PushClipRect(tl, &r);

	y = WIDGET(tl)->paddingTop;
	yLast = h;
	TAILQ_FOREACH(it, &tl->items, items) {
		const AG_TlistItem *itNext = TAILQ_NEXT(it,items);
		AG_Color *cSel;
		int *lbl, disabledItem;

		if (i++ < rOffs) {
			if (it->selected)
				selPos = -1;             /* For SCROLLTOSEL */

			if (itNext != NULL) {
				if (itNext->depth > it->depth) {
					SetExpansionLevel(tl, it->depth, 1);
				} else if (itNext->depth < it->depth) {
					SetExpansionLevel(tl, it->depth, 0);
				}
			}
			continue;                                /* Clipped */
		}
		if (y >= yLast) {                               /* Overflow */
			if (itNext == NULL) {
				break;                       /* End of list */
			}
			if (itNext->depth > it->depth) {
				SetExpansionLevel(tl, it->depth, y);
			} else if (itNext->depth < it->depth &&
			          (it->depth - itNext->depth) > 1) {
				/*
				 * Backtrack intermediate levels.
				 */
				for (j = itNext->depth;
				     j < (it->depth - 1);
				     j++) {
					AG_DrawLineV(tl,
					    (j * hItem) + hItem_2,     /* x */
					    y,                        /* y2 */
					    tl->expLevels[j+1],       /* y1 */
					    cBg);
				}
				SetExpansionLevel(tl, it->depth, 0);
			}
			y += hItem;
			continue;                                /* Clipped */
		}

		disabledItem = (disabled || (it->flags & AG_TLIST_ITEM_DISABLED));
		if (disabledItem) {
			lbl = &it->label[0];
			cSel = &WCOLOR_DISABLED(tl,SELECTION_COLOR);
		} else {
			if (it->selected) {
				lbl = &it->label[2];
				selSeen = 1;
			} else {
				lbl = &it->label[1];
			}
			cSel = &WCOLOR_DEFAULT(tl,SELECTION_COLOR);
		}

		if (*lbl == -1) {                      /* Render item label */
			AG_Surface *S, *Stext;
			AG_Color *cItemBg = (disabledItem) ?
			                    &WCOLOR_DISABLED(tl,BG_COLOR) :
			                    &WCOLOR_DEFAULT(tl,BG_COLOR);
			int wReq, x = paddingLeft, yAligned;
		
			if (it->color != NULL) {               /* Alt color */
				AG_TextColor(it->color);
			} else {
				AG_TextColor(disabledItem ?
				    &WCOLOR_DISABLED(tl,TEXT_COLOR) :
				    &WCOLOR_DEFAULT(tl,TEXT_COLOR));
			}
			if (it->font != NULL) {                 /* Alt font */
				AG_PushTextState();
				AG_TextFont(it->font);
				Stext = AG_TextRender(it->text);
				AG_PopTextState();
			} else if (it->fontFlags != 0) {  /* Alt font style */
				const AG_Font *fontOrig = WFONT(tl);

				AG_PushTextState();
				AG_TextFontLookup(OBJECT(fontOrig)->name,
				    fontOrig->spec.size,
				    it->fontFlags);
				Stext = AG_TextRender(it->text);
				AG_PopTextState();
			} else {
				Stext = AG_TextRender(it->text);
			}

			wReq = Stext->w;

			if (it->iconsrc)
				wReq += tl->icon_w + spacingHoriz;

			S = AG_SurfaceStdRGB(wReq, hItem);
#ifdef AG_DEBUG
			S->guides[0] = Stext->guides[0];
#endif
			AG_FillRect(S, NULL, it->selected ? cSel : cItemBg);

			if (it->iconsrc != NULL) {
				const AG_Surface *Sicon = it->iconsrc;

				if (Sicon->w > hItem || Sicon->h > hItem) {
					AG_Surface *SiconPr;

					SiconPr = AG_SurfaceScale(Sicon,
					    tl->icon_w, hItem, 0);
					if (SiconPr != NULL) {
						yAligned = hItem_2 -
						           (SiconPr->h >> 1);
						if (yAligned < 0)
							yAligned = 0;

						AG_SurfaceBlit(SiconPr, NULL,
						    S, x,yAligned);
						AG_SurfaceFree(SiconPr);
					}
					x += hItem + spacingHoriz;
				} else {
					yAligned = hItem_2 - (Sicon->h >> 1);
					if (yAligned < 0)
						yAligned = 0;

					AG_SurfaceBlit(Sicon, NULL, S,
					    x,yAligned);

					x += Sicon->w + spacingHoriz;
				}
			}

			yAligned = (Stext->guides[0] >> 1) - hItem_2;
			if (yAligned < 0)
				yAligned = 0;

			AG_SurfaceBlit(Stext, NULL, S, x,yAligned);
			AG_SurfaceFree(Stext);

			*lbl = AG_WidgetMapSurface(tl, S);
		}

		AG_WidgetBlitSurface(tl, *lbl,
		    paddingLeft + ((it->depth + 1)*hItem),
		    y + 1);

		if (it->selected) {   /* Fill in remaining BG to match label */
			AG_Rect rs;

			rs.x = (it->depth + 1)*hItem + WSURFACE(tl,*lbl)->w;
			rs.y = y + 1;
			rs.w = WIDTH(tl) - rs.x - WIDTH(tl->sbar);
			rs.h = hItem + 1;
			if (rs.w > 0) {
				AG_DrawRect(tl, &rs, cSel);
			}
			rs.x = (it->depth * hItem);
			rs.w = hItem + 1;
			AG_DrawRect(tl, &rs, cSel);
		}

		/*
		 * Tree lines (forward).
		 */
		if (it->depth > 0) {
			for (j = 0; j < it->depth - 1; j++) {
				if (!tl->expLevels[j]) {
					continue;
				}
				AG_DrawLineV(tl,
				    (j * hItem) + hItem_2,             /* x */
				    y,                                /* y1 */
				    y+hItem,                          /* y2 */
				    cLine);
			}
			if (itNext == NULL || itNext->depth < it->depth) {
				AG_DrawLineV(tl,
				    (it->depth - 1)*hItem + hItem_2,   /* x */
				    y,                                /* y1 */
				    y + hItem_2,                      /* y2 */
				    cLine);
			} else {
				AG_DrawLineV(tl,
				    (it->depth - 1)*hItem + hItem_2,   /* x */
				    y,                                /* y1 */
				    y + hItem,                        /* y2 */
				    cLine);
			}
		}

		/*
		 * Tree lines (backtracking).
		 */
		if (itNext != NULL) {
			if (itNext->depth > it->depth) {
				SetExpansionLevel(tl, it->depth, y+hItem_2+1);
			} else if (itNext->depth < it->depth) {
				if ((it->depth - itNext->depth) > 1) {
					for (j = itNext->depth;  /* Backtrack */
					     j < it->depth - 1;
					     j++) {
						AG_DrawLineV(tl,
						    j*hItem + hItem_2,   /* x */
						    tl->expLevels[j+1], /* y1 */
						    y + hItem,          /* y2 */
						    cBg);
					}
				}
				SetExpansionLevel(tl, it->depth, 0);
			}
		}

		AG_DrawLineH(tl,
		    ((it->depth - 1) * hItem) + (hItem >> 1),       /* x1 */
		    (    (it->depth) * hItem) + (hItem >> 1),       /* x2 */
		    y + (hItem >> 1),                               /* y */
		    cLine);

		if (it->flags & AG_TLIST_HAS_CHILDREN) {
			DrawExpColl(tl,it,
			    paddingLeft + (it->depth * hItem),
			    y);
		}

		y += hItem;

		AG_DrawLineH(tl, 0, (tl->r.w - 2), y, cBgLine);
	}

	if (!selSeen && (tl->flags & AG_TLIST_SCROLLTOSEL)) {
		if (selPos == -1) {
			tl->rOffs--;
		} else {
			tl->rOffs++;
		}
	} else {
		tl->flags &= ~(AG_TLIST_SCROLLTOSEL);
	}
	AG_PopClipRect(tl);
}

/* Draw Expand / Collapse control area. */
static void
DrawExpColl(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it, int x, int y)
{
	AG_Rect r;
	const AG_Color *cLine = &WCOLOR(tl, LINE_COLOR);
	static AG_VectorElement expdSign[] = {
		{ AG_VE_LINE,    3,5,  1,0, 0, NULL },            /* - */
		{ AG_VE_LINE,    1,7,  1,0, 0, NULL },            /* | */
	};
	const int h = tl->item_h >> 1;
	const int h_2 = (h >> 1);

	r.x = x + h_2;
	r.y = y + h_2;
	r.w = h + 1;
	r.h = h + 2;
	if ((h & 1) == 0) {
		r.w++;
		r.h++;
	}

	AG_DrawRectFilled(tl, &r, &WCOLOR(tl, FG_COLOR));

	if (it->flags & AG_TLIST_ITEM_EXPANDED) {
		AG_DrawVector(tl, 3,3, &r, cLine, expdSign, 0,1);    /* - */
	} else {
		AG_DrawVector(tl, 3,3, &r, cLine, expdSign, 0,2);    /* + */
	}
}

/* Remove a tlist item. */
void
AG_TlistDel(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_REMOVE(&tl->items, it, items);
	tl->nItems--;
	FreeItem(tl, it);

	/* Update the scrollbar range and offset accordingly. */
	if (tl->rOffs + tl->nVisible > tl->nItems) {
		tl->rOffs = MAX(0, tl->nItems - tl->nVisible);
	}

	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Remove duplicate items from the list. */
void
AG_TlistUniq(AG_Tlist *tl)
{
	AG_TlistItem *it, *it2;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);
restart:							/* XXX */
	TAILQ_FOREACH(it, &tl->items, items) {
		TAILQ_FOREACH(it2, &tl->items, items) {
			if (it != it2 &&
			    tl->compare_fn(it, it2) != 0) {
				AG_TlistDel(tl, it);
				goto restart;
			}
		}
	}
	AG_ObjectUnlock(tl);
}

/* Clear the items on the list, save the selections if polling. */
void
AG_TlistBegin(AG_Tlist *tl)
{
	AG_TlistItem *it, *nit;
	
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if ((!(tl->flags & AG_TLIST_NOSELSTATE) && it->selected) ||
		      (it->flags & AG_TLIST_HAS_CHILDREN)) {
			TAILQ_INSERT_HEAD(&tl->selitems, it, selitems);
		} else {
			FreeItem(tl, it);
		}
	}
	TAILQ_INIT(&tl->items);
	tl->nItems = 0;

	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Generic string compare routine. */
int
AG_TlistCompareStrings(const AG_TlistItem *a, const AG_TlistItem *b)
{
	return (strcmp(a->text, b->text) == 0);
}

/* Generic pointer compare routine. */
int
AG_TlistComparePtrs(const AG_TlistItem *a, const AG_TlistItem *b)
{
	return (a->p1 == b->p1);
}

/* Generic pointer+class compare routine. */
int
AG_TlistComparePtrsAndClasses(const AG_TlistItem *a,const AG_TlistItem *b)
{
	return ((a->p1 == b->p1) &&
	        (a->cat != NULL && b->cat != NULL &&
		 (strcmp(a->cat, b->cat) == 0)));
}

/* Set an alternate compare function for items. */
void
AG_TlistSetCompareFn(AG_Tlist *tl,
    int (*fn)(const AG_TlistItem *_Nonnull, const AG_TlistItem *_Nonnull))
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->compare_fn = fn;

	AG_ObjectUnlock(tl);
}

/* Restore previous item selection state. */
void
AG_TlistEnd(AG_Tlist *tl)
{
	AG_TlistItem *sit, *cit, *nsit;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	for (sit = TAILQ_FIRST(&tl->selitems);
	     sit != TAILQ_END(&tl->selitems);
	     sit = nsit) {
		nsit = TAILQ_NEXT(sit, selitems);
		TAILQ_FOREACH(cit, &tl->items, items) {
			if (!tl->compare_fn(sit, cit)) {
				continue;
			}
			if (!(tl->flags & AG_TLIST_NOSELSTATE)) {
				cit->selected = sit->selected;
			}
			if (sit->flags & AG_TLIST_ITEM_EXPANDED) {
				cit->flags |= AG_TLIST_ITEM_EXPANDED;
			} else {
				cit->flags &= ~(AG_TLIST_ITEM_EXPANDED);
			}
		}
		FreeItem(tl, sit);
	}
	TAILQ_INIT(&tl->selitems);

	AG_ObjectUnlock(tl);
}

/*
 * In the context of a polling routine, evaluate whether a newly-created
 * item should make its own child items visible based on the previously
 * saved state. If there are no items in the saved state which match the
 * newly-created item (according to the Compare function), then return TRUE
 * if the Tlist option EXPAND_NODES is set, otherwise return FALSE.
 */
int
AG_TlistVisibleChildren(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_TlistItem *itSaved;

	if ((it->flags & AG_TLIST_HAS_CHILDREN) == 0) {
		return (0);
	}
	AG_TAILQ_FOREACH(itSaved, &tl->selitems, selitems) {
		if (tl->compare_fn(itSaved, it))
			break;
	}
	if (itSaved == NULL) {
		return (tl->flags & AG_TLIST_EXPAND_NODES);  /* Default state */
	}
	return (itSaved->flags & AG_TLIST_ITEM_EXPANDED);      /* Saved state */
}

void
AG_TlistRefresh(AG_Tlist *_Nonnull tl)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->flags |= AG_TLIST_REFRESH;
	AG_Redraw(tl);

	AG_ObjectUnlock(tl);
}

static __inline__ void
InsertItemHead(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_ObjectLock(tl);

	TAILQ_INSERT_HEAD(&tl->items, it, items);
	tl->nItems++;

	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

static __inline__ void
InsertItemTail(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_ObjectLock(tl);

	TAILQ_INSERT_TAIL(&tl->items, it, items);
	tl->nItems++;

	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Add an item to the tail of the list (user pointer) */
AG_TlistItem *
AG_TlistAddPtr(AG_Tlist *tl, const AG_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");

	it = AG_TlistItemNew(icon);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));

	InsertItemTail(tl, it);
	return (it);
}

/* Add an item to the tail of the list (format string) */
AG_TlistItem *_Nonnull
AG_TlistAdd(AG_Tlist *tl, const AG_Surface *icon, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	
	it = AG_TlistItemNew(icon);
	it->p1 = it->text;
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);

	InsertItemTail(tl, it);
	return (it);
}

/* Add an item to the tail of the list (plain string) */
AG_TlistItem *
AG_TlistAddS(AG_Tlist *tl, const AG_Surface *icon, const char *text)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");

	it = AG_TlistItemNew(icon);
	it->p1 = it->text;
	Strlcpy(it->text, text, sizeof(it->text));

	InsertItemTail(tl, it);
	return (it);
}

/* Add an item to the head of the list (format string) */
AG_TlistItem *
AG_TlistAddHead(AG_Tlist *tl, const AG_Surface *icon, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	
	it = AG_TlistItemNew(icon);
	it->p1 = it->text;
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);

	InsertItemHead(tl, it);
	return (it);
}

/* Add an item to the head of the list (plain string) */
AG_TlistItem *
AG_TlistAddHeadS(AG_Tlist *tl, const AG_Surface *icon, const char *text)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");

	it = AG_TlistItemNew(icon);
	it->p1 = it->text;
	Strlcpy(it->text, text, sizeof(it->text));

	InsertItemHead(tl, it);
	return (it);
}

/* Add an item to the head of the list (user pointer) */
AG_TlistItem *
AG_TlistAddPtrHead(AG_Tlist *tl, const AG_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");

	it = AG_TlistItemNew(icon);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));

	InsertItemHead(tl, it);
	return (it);
}

/* Return a newly allocated and initialized AG_TlistItem */
AG_TlistItem *
AG_TlistItemNew(const AG_Surface *icon)
{
	AG_TlistItem *it;

	it = Malloc(sizeof(AG_TlistItem));
#ifdef AG_TYPE_SAFETY
	Strlcpy(it->tag, AG_TLIST_ITEM_TAG, sizeof(it->tag));
#endif
	it->label[2] = it->label[1] = it->label[0] = -1;
	it->cat = "";
	it->iconsrc = (icon) ? AG_SurfaceDup(icon) : NULL;

	memset(&it->p1, 0, sizeof(void *) +         /* p1 */
	                   sizeof(AG_Color *) +     /* color */
	                   sizeof(AG_Font *) +      /* font */
	                   sizeof(int) +            /* selected */
	                   sizeof(Uint) +           /* depth */
	                   sizeof(Uint) +           /* flags */
	                   sizeof(Uint) +           /* fontFlags */
	                   sizeof(char));           /* text[0] */
	return (it);
}

/* Set the graphical icon to display along with an item. */
void
AG_TlistSetIcon(AG_Tlist *tl, AG_TlistItem *it, const AG_Surface *S)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if (it->iconsrc) {
		AG_SurfaceFree(it->iconsrc);
	}
	it->iconsrc = (S) ? AG_SurfaceDup(S) : NULL;

	InvalidateLabels(tl, it);
	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Set an alternate, per-item text color. */
void
AG_TlistSetColor(AG_Tlist *tl, AG_TlistItem *it, const AG_Color *c)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if (it->color) {
		free(it->color);
	}
	if (c) {
		it->color = Malloc(sizeof(AG_Color));
		memcpy(it->color, c, sizeof(AG_Color));
	} else {
		it->color = NULL;
	}

	InvalidateLabels(tl, it);
	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Set an alternate, per-item font. */
void
AG_TlistSetFont(AG_Tlist *tl, AG_TlistItem *it, AG_Font *font)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);
#if 0
	if (it->font)
		AG_UnusedFont(it->font);
#endif
	if (font) {
		font->nRefs++;
		it->font = font;
	} else {
		it->font = NULL;
	}

	InvalidateLabels(tl, it);
	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Select an item based on its pointer value. */
AG_TlistItem *
AG_TlistSelectPtr(AG_Tlist *tl, void *p)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	UpdatePolled(tl);
	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->p1 == p) {
			SelectItem(tl, it);
			break;
		}
	}

	AG_ObjectUnlock(tl);
	return (it);
}

/* Select an item based on text. */
AG_TlistItem *
AG_TlistSelectText(AG_Tlist *tl, const char *text)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	UpdatePolled(tl);
	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->text[0] == text[0] &&
		    strcmp(it->text, text) == 0) {
			SelectItem(tl, it);
			break;
		}
	}

	AG_ObjectUnlock(tl);
	return (it);
}

/* Set the selection flag on an item (by reference). */
void
AG_TlistSelect(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	SelectItem(tl, it);

	AG_ObjectUnlock(tl);
}

/* Clear the selection flag on an item (by reference). */
void
AG_TlistDeselect(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	DeselectItem(tl, it);

	AG_ObjectUnlock(tl);
}

/* Set the selection flag on an item (by index). */
void
AG_TlistSelectIdx(AG_Tlist *tl, Uint idx)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	if (idx == 0) {
		if (!TAILQ_EMPTY(&tl->items))
			SelectItem(tl, TAILQ_FIRST(&tl->items));
	} else {
		AG_TlistItem *it;
		Uint i=0;

		TAILQ_FOREACH(it, &tl->items, items) {
			if (i++ == idx) {
				SelectItem(tl, it);
				break;
			}
		}
	}

	AG_ObjectUnlock(tl);
}

/* Set the selection flag on an item (by index). */
void
AG_TlistDeselectIdx(AG_Tlist *tl, Uint idx)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	if (idx == 0) {
		if (!TAILQ_EMPTY(&tl->items))
			DeselectItem(tl, TAILQ_FIRST(&tl->items));
	} else {
		AG_TlistItem *it;
		Uint i=0;

		TAILQ_FOREACH(it, &tl->items, items) {
			if (i++ == idx) {
				DeselectItem(tl, it);
				break;
			}
		}
	}

	AG_ObjectUnlock(tl);
}

/* Set the selection flag on all items. */
void
AG_TlistSelectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items)
		SelectItem(tl, it);

	AG_ObjectUnlock(tl);
}

/* Unset the selection flag on all items. */
void
AG_TlistDeselectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items)
		DeselectItem(tl, it);

	AG_ObjectUnlock(tl);
}

static void
PopupMenu(AG_Tlist *_Nonnull tl, AG_TlistPopup *_Nonnull tp, int x, int y)
{
	AG_Menu *m = tp->menu;
	
#if 0
	if (AG_ParentWindow(tl) == NULL)
		AG_FatalError("AG_Tlist: Unattached");
#endif
	if (tp->panel) {
		AG_MenuCollapse(tp->item);
		tp->panel = NULL;
	}
	m->itemSel = tp->item;
	tp->panel = AG_MenuExpand(tl, tp->item, x+4, y+4);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2) - WIDGET(tl)->paddingLeft;
	const int y = AG_INT(3) - WIDGET(tl)->paddingTop;
	AG_TlistItem *ti;
	const int idx = tl->rOffs + y/tl->item_h + 1;
	
	if (!AG_WidgetIsFocused(tl))
		AG_WidgetFocus(tl);

	switch (button) {
	case AG_MOUSE_WHEELUP:
		tl->rOffs -= AG_GetInt(tl,"line-scroll-amount");
		if (tl->rOffs < 0) {
			tl->rOffs = 0;
		}
		AG_Redraw(tl);
		break;
	case AG_MOUSE_WHEELDOWN:
		tl->rOffs += AG_GetInt(tl,"line-scroll-amount");
		if (tl->rOffs > (tl->nItems - tl->nVisible)) {
			tl->rOffs = MAX(0, tl->nItems - tl->nVisible);
		}
		AG_Redraw(tl);
		break;
	}

	if (x > WIDTH(tl) - WIDTH(tl->sbar))
		return;

	if ((ti = AG_TlistFindByIndex(tl, idx)) == NULL)
		return;

	switch (button) {
	case AG_MOUSE_LEFT:
	case AG_MOUSE_RIGHT:
		if (ti->flags & AG_TLIST_HAS_CHILDREN) {       /* [+] control */
			if (x >= (ti->depth)*tl->icon_w &&
			    x <= (ti->depth+1)*tl->icon_w) {
				if (ti->flags & AG_TLIST_ITEM_EXPANDED) {
					ti->flags &= ~AG_TLIST_ITEM_EXPANDED;
				} else {
					ti->flags |=  AG_TLIST_ITEM_EXPANDED;
				}
				tl->flags |= AG_TLIST_REFRESH;
				AG_Redraw(tl);
				break;
			}
		}

		if (ti->flags & AG_TLIST_NO_SELECT)
			break;

		if ((tl->flags & AG_TLIST_MULTI) &&
		    (AG_GetModState(tl) & AG_KEYMOD_SHIFT)) {
			SelectRange(tl, idx);
		}
		if ((tl->flags & AG_TLIST_MULTITOGGLE) ||
		    ((tl->flags & AG_TLIST_MULTI) &&
		     (AG_GetModState(tl) & AG_KEYMOD_CTRL))) {
			if (ti->selected) {
				DeselectItem(tl, ti);
			} else {
				SelectItem(tl, ti);
			}
			break;
		}

		AG_TlistDeselectAll(tl);
		SelectItem(tl, ti);

		break;
	}

	switch (button) {
	case AG_MOUSE_LEFT:
		if (tl->dblClicked && tl->dblClicked == ti->p1) {
			AG_DelTimer(tl, &tl->dblClickTo);
			if (tl->dblClickEv) {
				AG_PostEventByPtr(tl, tl->dblClickEv, "%p", ti);
			}
			AG_PostEvent(tl, "tlist-dblclick", "%p", ti);
			tl->dblClicked = NULL;
		} else {
			tl->dblClicked = ti->p1;
			AG_AddTimer(tl, &tl->dblClickTo, agMouseDblclickDelay,
			            DoubleClickTimeout, NULL);
		}
		break;
	case AG_MOUSE_RIGHT:
		if ((ti->flags & AG_TLIST_NO_POPUP) == 0) {
			if (tl->popupEv) {
				AG_PostEventByPtr(tl, tl->popupEv, NULL);
			} else if (ti->cat) {
				AG_TlistPopup *tp;
		
				if (!(tl->flags &
				    (AG_TLIST_MULTITOGGLE | AG_TLIST_MULTI)) ||
				    !(AG_GetModState(tl) & (AG_KEYMOD_CTRL |
				                            AG_KEYMOD_SHIFT))) {
					AG_TlistDeselectAll(tl);
					SelectItem(tl, ti);
				}
				TAILQ_FOREACH(tp, &tl->popups, popups) {
					if (strcmp(tp->iclass, ti->cat) == 0)
						break;
				}
				if (tp)
					PopupMenu(tl, tp, x,y);
			}
		}
	}
}

/* Handle multiple selections (shift) */
static void
SelectRange(AG_Tlist *tl, int idx)
{
	AG_TlistItem *oitem;
	int idxOther = -1;
	int i = 0, nItems = 0;

	TAILQ_FOREACH(oitem, &tl->items, items) {
		if (oitem->selected) {
			idxOther = i;
		}
		i++;
		nItems++;
	}
	if (idxOther == -1) {
		return;
	}
	if (idxOther < idx) {			  /* Forward */
		i = 0;
		TAILQ_FOREACH(oitem, &tl->items, items) {
			if (i == idx)
				break;
			if (i > idxOther) {
				SelectItem(tl, oitem);
			}
			i++;
		}
	} else if (idxOther >= idx) {		  /* Backward */
		i = nItems;
		TAILQ_FOREACH_REVERSE(oitem, &tl->items,
		    ag_tlist_itemq, items) {
			if (i <= idxOther)
				SelectItem(tl, oitem);
			if (i == idx)
				break;
			i--;
		}
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const int keysym = AG_INT(1);
	void *ti;

	switch (keysym) {
	case AG_KEY_UP:
		DecrementSelection(tl, 1);
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", -1);
		break;
	case AG_KEY_DOWN:
		IncrementSelection(tl, 1);
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", +1);
		break;
	case AG_KEY_PAGEUP:
		DecrementSelection(tl, agPageIncrement);
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", -agPageIncrement);
		break;
	case AG_KEY_PAGEDOWN:
		IncrementSelection(tl, agPageIncrement);
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", +agPageIncrement);
		break;
	case AG_KEY_HOME:
		AG_TlistScrollToStart(tl);
		break;
	case AG_KEY_END:
		AG_TlistScrollToEnd(tl);
		break;
	case AG_KEY_RETURN:
	case AG_KEY_KP_ENTER:
		if ((ti = AG_TlistSelectedItemPtr(tl)) != NULL) {
			AG_PostEvent(tl, "tlist-return", "%p", ti);
		}
		break;
	}
	tl->lastKeyDown = keysym;
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_PAGEUP:
	case AG_KEY_PAGEDOWN:
		if (keysym == tl->lastKeyDown) {
			AG_DelTimer(tl, &tl->moveTo);
		}
		break;
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Tlist *tl = obj;

	WIDGET(tl)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_TEXT;

	tl->flags = 0;
	tl->item_h = agTextFontHeight + AG_TLIST_PADDING;
	if (!(tl->item_h & 1)) {
		tl->item_h++;
	}
	tl->selected = NULL;
	tl->wHint = 0;
	tl->hHint = tl->item_h + 2;
	tl->r.x = 0;
	tl->r.y = 0;
	tl->r.w = 0;
	tl->r.h = 0;

	tl->nExpLevels = AG_TLIST_EXP_LEVELS_INIT;
	tl->expLevels = Malloc(tl->nExpLevels * sizeof(int));

	tl->icon_w = tl->item_h + 1;
	tl->pollDelay = 1000;
	tl->rOffs = 0;
	tl->dblClicked = NULL;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	tl->nItems = 0;
	tl->nVisible = 0;
	TAILQ_INIT(&tl->popups);
	tl->compare_fn = AG_TlistComparePtrs;
	tl->popupEv = NULL;
	tl->changedEv = NULL;
	tl->dblClickEv = NULL;
	tl->lastKeyDown = AG_KEY_NONE;

	AG_InitTimer(&tl->moveTo, "move", 0);
	AG_InitTimer(&tl->refreshTo, "refresh", 0);
	AG_InitTimer(&tl->dblClickTo, "dblClick", 0);

	tl->sbar = AG_ScrollbarNew(tl, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_SetInt(tl->sbar, "min", 0);
	AG_BindInt(tl->sbar, "max", &tl->nItems);
	AG_BindInt(tl->sbar, "visible", &tl->nVisible);
	AG_BindInt(tl->sbar, "value", &tl->rOffs);
	AG_WidgetSetFocusable(tl->sbar, 0);
	
	AG_SetInt(tl, "line-scroll-amount", 5);

	AG_AddEvent(tl, "font-changed", FontChanged, NULL);
	AG_AddEvent(tl, "palette-changed", FontChanged, NULL);
	AG_SetEvent(tl, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tl, "key-down", KeyDown, NULL);
	AG_AddEvent(tl, "widget-shown", OnShow, NULL);
	AG_AddEvent(tl, "widget-hidden", OnHide, NULL);
	AG_SetEvent(tl, "widget-lostfocus", OnLostFocus, NULL);
	AG_SetEvent(tl, "key-up", KeyUp, NULL);

	AG_BindPointer(tl, "selected", &tl->selected);
}

/*
 * Return the item at the given index. Result is only valid as long as
 * the Tlist is locked.
 */
AG_TlistItem *
AG_TlistFindByIndex(AG_Tlist *tl, int index)
{
	AG_TlistItem *it;
	int i = 0;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (++i == index) {
			AG_ObjectUnlock(tl);
			return (it);
		}
	}

	AG_ObjectUnlock(tl);
	return (NULL);
}

/*
 * Return the first selected item. Result is only valid as long as the Tlist
 * is locked.
 */
AG_TlistItem *
AG_TlistSelectedItem(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			AG_ObjectUnlock(tl);
			return (it);
		}
	}

	AG_ObjectUnlock(tl);
	return (NULL);
}

/*
 * Return the user pointer of the first selected item. Result is only valid
 * as long as the Tlist is locked.
 */
void *
AG_TlistSelectedItemPtr(AG_Tlist *tl)
{
	AG_TlistItem *it;
	void *rv;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			rv = it->p1;
			AG_ObjectUnlock(tl);
			return (rv);
		}
	}

	AG_ObjectUnlock(tl);
	return (NULL);
}

/*
 * Return the pointer associated with the first selected item. Result is only
 * valid as long as the Tlist is locked.
 */
void *
AG_TlistFindPtr(AG_Tlist *tl)
{
	AG_TlistItem *it;
	void *rv;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			rv = it->p1;
			AG_ObjectUnlock(tl);
			return (rv);
		}
	}

	AG_ObjectUnlock(tl);
	return (NULL);
}

/*
 * Return the first item matching a text string. Result is only valid as long
 * as the Tlist is locked.
 */
AG_TlistItem *
AG_TlistFindText(AG_Tlist *tl, const char *text)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (strcmp(it->text, text) == 0) {
			AG_ObjectUnlock(tl);
			return (it);
		}
	}

	AG_ObjectUnlock(tl);
	return (NULL);
}

/*
 * Return the first item on the list. Result is only valid as long as
 * the Tlist is locked.
 */
AG_TlistItem *
AG_TlistFirstItem(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	it = TAILQ_FIRST(&tl->items);

	AG_ObjectUnlock(tl);
	return (it);
}

/*
 * Return the last item on the list. Result is only valid as long as
 * the Tlist is locked.
 */
AG_TlistItem *
AG_TlistLastItem(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	it = TAILQ_LAST(&tl->items, ag_tlist_itemq);

	AG_ObjectUnlock(tl);
	return (it);
}

/* Set the height to use for item display. */
void
AG_TlistSetItemHeight(AG_Tlist *tl, int ih)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->item_h = ih;

	TAILQ_FOREACH(it, &tl->items, items) {
		InvalidateLabels(tl, it);
	}
	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Set the width to use for item icons. */
void
AG_TlistSetIconWidth(AG_Tlist *tl, int iw)
{
	AG_TlistItem *it;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->icon_w = iw;

	TAILQ_FOREACH(it, &tl->items, items) {
		InvalidateLabels(tl, it);
	}
	AG_Redraw(tl);
	AG_ObjectUnlock(tl);
}

/* Set a callback to run when the user double clicks on an item. */
void
AG_TlistSetDblClickFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->dblClickEv = AG_SetEvent(tl, NULL, fn, NULL);

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(tl->dblClickEv, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(tl);
}

/* Set a callback to run when the user right-clicks on an item. */
void
AG_TlistSetPopupFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->popupEv = AG_SetEvent(tl, NULL, fn, NULL);

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(tl->popupEv, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(tl);
}

/* Set a callback to run when the selection changes. */
void
AG_TlistSetChangedFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	tl->changedEv = AG_SetEvent(tl, NULL, fn, NULL);

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(tl->changedEv, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(tl);
}

/* Create a new popup menu for items of the given class. */
AG_MenuItem *
AG_TlistSetPopup(AG_Tlist *tl, const char *iclass)
{
	AG_TlistPopup *tp;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");

	tp = Malloc(sizeof(AG_TlistPopup));
	tp->iclass = iclass;
	tp->panel = NULL;
	tp->menu = AG_MenuNew(NULL, 0);
	tp->item = tp->menu->root;		/* XXX redundant */

	/* AG_MenuExpand() need a window pointer in AG_Menu */
	WIDGET(tp->menu)->window = WIDGET(tl)->window;

	AG_ObjectLock(tl);
	TAILQ_INSERT_TAIL(&tl->popups, tp, popups);
	AG_ObjectUnlock(tl);
	return (tp->item);
}

/* Scroll to the beginning of the list. */
void
AG_TlistScrollToStart(AG_Tlist *tl)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	tl->rOffs = 0;
	AG_Redraw(tl);
}

/* Scroll to the end of the list. */
void
AG_TlistScrollToEnd(AG_Tlist *tl)
{
	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	tl->rOffs = MAX(0, tl->nItems - tl->nVisible);
	AG_Redraw(tl);
}

static int
CompareText(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const AG_TlistItem *it1 = *(const AG_TlistItem **)p1;
	const AG_TlistItem *it2 = *(const AG_TlistItem **)p2;

	return strcoll(it1->text, it2->text);
}

/* Sort list items by text using quicksort. */
void
AG_TlistSort(AG_Tlist *tl)
{
	AG_TlistItem *it, **items;
	Uint i = 0;

	if ((items = TryMalloc(tl->nItems * sizeof(AG_TlistItem *))) == NULL)
		return;

	AG_OBJECT_ISA(tl, "AG_Widget:AG_Tlist:*");
	AG_ObjectLock(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		items[i++] = it;
	}
	qsort(items, tl->nItems, sizeof(AG_TlistItem *), CompareText);
	TAILQ_INIT(&tl->items);
	for (i = 0; i < tl->nItems; i++)
		TAILQ_INSERT_TAIL(&tl->items, items[i], items);

	AG_Redraw(tl);
	AG_ObjectUnlock(tl);

	free(items);
}

#ifdef AG_TYPE_SAFETY
/*
 * Accessor for AG_[CONST_]TLIST_ITEM_PTR().
 */
AG_TlistItem *
AG_TlistGetItemPtr(const AG_Event *event, int idx, int isConst)
{
	const AG_Variable *V = &event->argv[idx];

	if (idx > event->argc || V->type != AG_VARIABLE_POINTER) {
		AG_GenericMismatch("by AG_TLIST_ITEM_PTR(idx)");
	}
	if (isConst) {
		if ((V->info.pFlags & AG_VARIABLE_P_READONLY) == 0)
			AG_FatalError("AG_TLIST_CONST_ITEM_PTR() argument isn't const. "
			              "Did you mean AG_TLIST_ITEM_PTR()?");
	} else {
		if (V->info.pFlags & AG_VARIABLE_P_READONLY)
			AG_FatalError("AG_TLIST_ITEM_PTR() argument is const. "
			              "Did you mean AG_CONST_TLIST_ITEM_PTR()?");
	}
	if (V->data.p == NULL) {
		return (NULL);
	}
	if (strncmp(AGTLISTITEM(V->data.p)->tag, AG_TLIST_ITEM_TAG, AG_TLIST_ITEM_TAG_LEN) != 0) {
		AG_GenericMismatch("by AG_TLIST_ITEM_PTR(tag)");
	}
	return (V->data.p);
}
#endif /* AG_TYPE_SAFETY */

AG_WidgetClass agTlistClass = {
	{
		"Agar(Widget:Tlist)",
		sizeof(AG_Tlist),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
