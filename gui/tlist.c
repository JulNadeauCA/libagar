/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/tlist.h>
#include <agar/gui/primitive.h>

#include <string.h>
#include <stdarg.h>

#ifndef AG_TLIST_PADDING
#define AG_TLIST_PADDING 2	/* Label padding (pixels) */
#endif

static void DrawExpandCollapseSign(AG_Tlist *_Nonnull, AG_TlistItem *_Nonnull,
                                   int, int);
static void StylizeFont(AG_Tlist *_Nonnull, Uint);

AG_Tlist *
AG_TlistNew(void *parent, Uint flags)
{
	AG_Tlist *tl;

	tl = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(tl, &agTlistClass);
	tl->flags |= flags;

	if (flags & AG_TLIST_HFILL) { AG_ExpandHoriz(tl); }
	if (flags & AG_TLIST_VFILL) { AG_ExpandVert(tl); }

	AG_ObjectAttach(parent, tl);
	return (tl);
}

#ifdef AG_TIMERS
AG_Tlist *
AG_TlistNewPolled(void *parent, Uint flags, AG_EventFn fn, const char *fmt, ...)
{
	AG_Tlist *tl;
	AG_Event *ev;

	tl = AG_TlistNew(parent, flags);
	AG_ObjectLock(tl);
	tl->flags |= AG_TLIST_POLL;
	ev = AG_SetEvent(tl, "tlist-poll", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	AG_ObjectUnlock(tl);
	AG_RedrawOnTick(tl, 1000);
	return (tl);
}

/* In AG_TLIST_POLL mode, invoke `tlist-poll' if refresh timer has expired. */
static __inline__ void
UpdatePolled(AG_Tlist *_Nonnull tl)
{
	if ((tl->flags & AG_TLIST_POLL) &&
	    (tl->flags & AG_TLIST_REFRESH)) {
		tl->flags &= ~(AG_TLIST_REFRESH);
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
}
#endif /* AG_TIMERS */

/* Return 1 if at least one selected item is visible */
static int
SelectionVisible(AG_Tlist *_Nonnull tl)
{
	AG_TlistItem *it;
	int y=0, i=0, rOffs, yLast, item_h;

#ifdef AG_TIMERS
	UpdatePolled(tl);
#endif
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
		tl->rOffs = (tl->rOffs > m) ? m : MAX(0, m - tl->nvisitems+1);
		AG_Redraw(tl);
		return;
	}
}

static void
SelectItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_Variable *selectedb;
	void **sel_ptr;

	selectedb = AG_GetVariable(tl, "selected", &sel_ptr);
	*sel_ptr = it->p1;
	if (!it->selected) {
		it->selected = 1;
		if (tl->changedEv != NULL) {
			AG_PostEventByPtr(NULL, tl, tl->changedEv, "%p,%i",
			    it, 1);
		}
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 1);
	}
	AG_PostEvent(NULL, tl, "tlist-selected", "%p", it);
	AG_UnlockVariable(selectedb);
	AG_Redraw(tl);
}

static void
DeselectItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	AG_Variable *selectedb;
	void **sel_ptr;

	selectedb = AG_GetVariable(tl, "selected", &sel_ptr);
	*sel_ptr = NULL;
	if (it->selected) {
		it->selected = 0;
		if (tl->changedEv != NULL) {
			AG_PostEventByPtr(NULL, tl, tl->changedEv, "%p,%i",
			    it, 0);
		}
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 0);
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
			itPrev = TAILQ_PREV(it, ag_tlist_itemq, items);
			if (itPrev != NULL) {
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
			if (!it->selected) {
				continue;
			}
			itNext = TAILQ_NEXT(it, items);
			if (itNext != NULL) {
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

#ifdef AG_TIMERS
static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	tl->dblClicked = NULL;
	return (0);
}

static void
OnFocusLoss(AG_Event *_Nonnull event)
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
#endif /* AG_TIMERS */

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != -1) {
			AG_WidgetUnmapSurface(tl, it->icon);
			it->icon = -1;
		}
		if (it->label != -1) {
			AG_WidgetUnmapSurface(tl, it->label);
			it->label = -1;
		}
	}
	AG_TlistSetItemHeight(tl, WIDGET(tl)->font->height + AG_TLIST_PADDING);
	AG_TlistSetIconWidth(tl, tl->item_h + 1);
}

#ifdef AG_TIMERS
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	if (tl->flags & AG_TLIST_POLL) {
		tl->flags |= AG_TLIST_REFRESH;
		AG_AddTimer(tl, &tl->refreshTo, 125, PollRefreshTimeout, NULL);
	}
}

/* Timer for moving keyboard selection. */
static Uint32
MoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	int incr = AG_INT(1);

	if (incr < 0) {
		DecrementSelection(tl, -incr);
	} else {
		IncrementSelection(tl, incr);
	}
	return (agKbdRepeat);
}
#endif /* AG_TIMERS */

void
AG_TlistSizeHint(AG_Tlist *tl, const char *text, int nitems)
{
	AG_ObjectLock(tl);
	AG_TextSize(text, &tl->wHint, NULL);
	tl->hHint = (tl->item_h+2)*nitems;
	AG_ObjectUnlock(tl);
}

void
AG_TlistSizeHintPixels(AG_Tlist *tl, int w, int nitems)
{
	AG_ObjectLock(tl);
	tl->wHint = w;
	tl->hHint = (tl->item_h+2)*nitems;
	AG_ObjectUnlock(tl);
}

/*
 * Set the default size hint to accomodate the largest text label in
 * the current list of items, and the given number of items.
 */
void
AG_TlistSizeHintLargest(AG_Tlist *tl, int nitems)
{
	AG_TlistItem *it;
	int w;

	AG_ObjectLock(tl);
#ifdef AG_TIMERS
	UpdatePolled(tl);
#endif
	tl->wHint = 0;
	AG_TLIST_FOREACH(it, tl) {
		AG_TextSize(it->text, &w, NULL);
		if (w > tl->wHint) { tl->wHint = w; }
	}
	tl->wHint += (tl->icon_w << 2);
	tl->hHint = (tl->item_h+2)*nitems;
	AG_ObjectUnlock(tl);
}

static void
FreeItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it)
{
	if (it->iconsrc != NULL)
		AG_SurfaceFree(it->iconsrc);
	if (it->icon != -1)
		AG_WidgetUnmapSurface(tl, it->icon);
	if (it->label != -1)
		AG_WidgetUnmapSurface(tl, it->label);

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
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Tlist *tl = obj;
	AG_SizeReq rBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);
	r->w = tl->icon_w + (tl->wSpace << 1) + tl->wHint + rBar.w;
	r->h = tl->hHint;
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
	aBar.y = 0;
	AG_WidgetSizeAlloc(tl->sbar, &aBar);
	tl->wRow = a->w - aBar.w;

	tl->r.w = tl->wRow;
	tl->r.h = a->h;

	/* Limit vertical scrollbar parameters */
	tl->nvisitems = a->h/tl->item_h;
	if (tl->rOffs+tl->nvisitems >= tl->nitems) {
		tl->rOffs = MAX(0, tl->nitems - tl->nvisitems);
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Tlist *tl = obj;
	AG_TlistItem *it;
	AG_Rect r;
	AG_Color cSel, cLine;
	const int wSpace = tl->wSpace;
	const int hItem = tl->item_h;
	const int wIcon = tl->icon_w;
	const int wRow = tl->wRow;
	const int rOffs = tl->rOffs;
	const int zoomLvl = WIDGET(tl)->window->zoom;
	int x, y=0, i=0, selSeen=0, selPos=1, h=HEIGHT(tl), yLast;

#ifdef AG_TIMERS
	UpdatePolled(tl);
#endif
	AG_DrawBox(tl, &tl->r, -1, &WCOLOR(tl,AG_BG_COLOR));
	cSel = WCOLOR_SEL(tl,AG_BG_COLOR);
	cLine = WCOLOR(tl,AG_LINE_COLOR);
	AG_WidgetDraw(tl->sbar);
	AG_PushClipRect(tl, &tl->r);

	if (zoomLvl < AG_ZOOM_1_1) {
		int z;
	
		/* Tint the line color to create an illusion of distance. */
		/* TODO combine these additions */
		for (z = zoomLvl; z < AG_ZOOM_1_1; z++)
			AG_ColorAdd(&cLine, &cLine, &agTint);
	}

	yLast = h;
	TAILQ_FOREACH(it, &tl->items, items) {
		if (i++ < rOffs) {
			if (it->selected) {
				selPos = -1;
			}
			continue;
		}
		if (y > yLast)
			break;

		x = wIcon * it->depth;

		if (it->selected) {
		    	AG_Rect rSel;

			rSel.x = x + wIcon;
			rSel.y = y + 1;
			rSel.w = wRow - x - wIcon - 1;
			rSel.h = hItem;
			AG_DrawRect(tl, &rSel, &cSel);
			selSeen = 1;
		}
		if (it->iconsrc != NULL) {
			if (it->icon == -1) {
				AG_Surface *S;

				if ((S = AG_SurfaceScale(it->iconsrc,
				    wIcon, hItem, 0)) == NULL) {
					AG_FatalError(NULL);
				}
				it->icon = AG_WidgetMapSurface(tl, S);
			}
			AG_WidgetBlitSurface(tl, it->icon, x,y);

			if (it->selected) {
				cSel.a >>= 1;
				r.x = x;
				r.y = y;
				r.w = wIcon;
				r.h = hItem;
				AG_DrawRectBlended(tl, &r, &cSel,
				    AG_ALPHA_SRC,
				    AG_ALPHA_ONE_MINUS_SRC);
				cSel.a <<= 1;
			}
		}
		if (it->flags & AG_TLIST_HAS_CHILDREN) {
			DrawExpandCollapseSign(tl,it, x,y);
		}
		if (it->label == -1) {
			int altFont = 0;

			if (it->color) {
				AG_TextColor(it->color);
			} else {
				AG_TextColor(it->selected ?
				             &WCOLOR_SEL(tl,AG_TEXT_COLOR) :
					     &WCOLOR(tl,AG_TEXT_COLOR));
			}
			if (it->font) {
				AG_PushTextState();
				AG_TextFont(it->font);
				altFont = 1;
			} else if (it->flags & AG_TLIST_ITEM_STYLE) {
				AG_PushTextState();
				StylizeFont(tl, it->flags);
				altFont = 1;
			}
			it->label = AG_WidgetMapSurface(tl,
			    AG_TextRender(it->text));

			if (altFont)
				AG_PopTextState();
		}

		AG_WidgetBlitSurface(tl, it->label,
		    x + wIcon + wSpace,
		    y + AG_TLIST_PADDING);
		
		y += hItem;
		
		if (y < h)
			AG_DrawLineH(tl, 0, wRow-2, y, &cLine);
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

static void
DrawExpandCollapseSign(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it,
    int x, int y)
{
	AG_Rect r;
	const AG_Color *cLine = &WCOLOR(tl,LINE_COLOR);
	static AG_VectorElement expdSign[] = {
		{ AG_VE_LINE,    3,5,  1,0, 0, NULL },            /* - */
		{ AG_VE_LINE,    1,7,  1,0, 0, NULL },            /* | */
	};

	r.x = x+1;
	r.y = y+1;
	r.h = r.w = tl->item_h;

	AG_DrawRectFilled(tl, &r, &WCOLOR(tl, AG_BG_COLOR));

	if (it->flags & AG_TLIST_ITEM_EXPANDED) {
		AG_DrawVector(tl, 3,3, &r, cLine, expdSign, 0,1);    /* - */
	} else {
		AG_DrawVector(tl, 3,3, &r, cLine, expdSign, 0,2);    /* + */
	}
}

static void
StylizeFont(AG_Tlist *_Nonnull tl, Uint itFlags)
{
	AG_Font *defFont = WIDGET(tl)->font;
	Uint fontFlags = 0;

	if (itFlags & AG_TLIST_ITEM_BOLD) { fontFlags |= AG_FONT_BOLD; }
	if (itFlags & AG_TLIST_ITEM_ITALIC) { fontFlags |= AG_FONT_ITALIC; }
	if (itFlags & AG_TLIST_ITEM_UNDERLINE) { fontFlags |= AG_FONT_UNDERLINE; }
	if (itFlags & AG_TLIST_ITEM_UPPERCASE) { fontFlags |= AG_FONT_UPPERCASE; }

	AG_TextFontLookup(OBJECT(defFont)->name, &defFont->spec.size, fontFlags);
}

/* Remove a tlist item. */
void
AG_TlistDel(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_ObjectLock(tl);
	TAILQ_REMOVE(&tl->items, it, items);
	tl->nitems--;
	FreeItem(tl, it);

	/* Update the scrollbar range and offset accordingly. */
	if (tl->rOffs+tl->nvisitems > tl->nitems) {
		tl->rOffs = MAX(0, tl->nitems - tl->nvisitems);
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

/* Remove duplicate items from the list. */
void
AG_TlistUniq(AG_Tlist *tl)
{
	AG_TlistItem *it, *it2;

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
	tl->nitems = 0;
	AG_ObjectUnlock(tl);

	AG_Redraw(tl);
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
	AG_ObjectLock(tl);
	tl->compare_fn = fn;
	AG_ObjectUnlock(tl);
}

#ifdef AG_TIMERS
/* Set the update rate for polled displays in ms (-1 = update explicitely). */
void
AG_TlistSetRefresh(AG_Tlist *tl, int ms)
{
	AG_ObjectLock(tl);
	if (ms == -1) {
		AG_DelTimer(tl, &tl->refreshTo);
	} else {
		AG_AddTimer(tl, &tl->refreshTo, ms, PollRefreshTimeout, NULL);
	}
	AG_ObjectUnlock(tl);
}
#endif /* AG_TIMERS */

/* Restore previous item selection state. */
void
AG_TlistEnd(AG_Tlist *tl)
{
	AG_TlistItem *sit, *cit, *nsit;

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

void
AG_TlistRestore(AG_Tlist *tl)
{
	AG_TlistEnd(tl);
}

void
AG_TlistClear(AG_Tlist *tl)
{
	AG_TlistBegin(tl);
}

int
AG_TlistVisibleChildren(AG_Tlist *tl, AG_TlistItem *cit)
{
	AG_TlistItem *sit;

	AG_TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tl->compare_fn(sit, cit))
			break;
	}
	if (sit == NULL) { 
		return (0);			/* TODO default setting */
	}
	return (sit->flags & AG_TLIST_ITEM_EXPANDED);
}

void
AG_TlistRefresh(AG_Tlist *_Nonnull tl)
{
	AG_ObjectLock(tl);
	tl->flags |= AG_TLIST_REFRESH;
	AG_ObjectUnlock(tl);
}

/* Return a newly allocated and initialized AG_TlistItem */
AG_TlistItem *
AG_TlistItemNew(AG_Tlist *_Nonnull tl, const AG_Surface *icon)
{
	AG_TlistItem *it;

	it = Malloc(sizeof(AG_TlistItem));
#ifdef AG_TYPE_SAFETY
	Strlcpy(it->tag, AG_TLIST_ITEM_TAG, sizeof(it->tag));
#endif
	it->selected = 0;
	it->icon = -1;
	it->iconsrc = (icon != NULL) ? AG_SurfaceDup(icon) : NULL;
	it->p1 = NULL;
	it->cat = "";
	it->label = -1;
	it->depth = 0;
	it->flags = 0;
	it->text[0] = '\0';
	it->color = NULL;
	it->font = NULL;
	return (it);
}

/* The Tlist must be locked. */
static __inline__ void
InsertItem(AG_Tlist *_Nonnull tl, AG_TlistItem *_Nonnull it, int ins_head)
{
	if (ins_head) {
		TAILQ_INSERT_HEAD(&tl->items, it, items);
	} else {
		TAILQ_INSERT_TAIL(&tl->items, it, items);
	}
	tl->nitems++;

	AG_Redraw(tl);
}

/* Add an item to the tail of the list (user pointer) */
AG_TlistItem *
AG_TlistAddPtr(AG_Tlist *tl, const AG_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the tail of the list (format string) */
AG_TlistItem *_Nonnull
AG_TlistAdd(AG_Tlist *tl, const AG_Surface *icon, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;
	
	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the tail of the list (plain string) */
AG_TlistItem *
AG_TlistAddS(AG_Tlist *tl, const AG_Surface *icon, const char *text)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (format string) */
AG_TlistItem *
AG_TlistAddHead(AG_Tlist *tl, const AG_Surface *icon, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;
	
	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (plain string) */
AG_TlistItem *
AG_TlistAddHeadS(AG_Tlist *tl, const AG_Surface *icon, const char *text)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (user pointer) */
AG_TlistItem *
AG_TlistAddPtrHead(AG_Tlist *tl, const AG_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AG_TlistItemNew(tl, icon);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Set the graphical icon to display along with an item. */
void
AG_TlistSetIcon(AG_Tlist *tl, AG_TlistItem *it, const AG_Surface *S)
{
	AG_ObjectLock(tl);
	if (it->iconsrc != NULL) {
		AG_SurfaceFree(it->iconsrc);
	}
	it->iconsrc = (S != NULL) ? AG_SurfaceDup(S) : NULL;
	if (it->icon != -1) {
		AG_WidgetUnmapSurface(tl, it->icon);
		it->icon = -1;
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

/* Set an alternate, per-item text color. */
void
AG_TlistSetColor(AG_Tlist *tl, AG_TlistItem *it, const AG_Color *c)
{
	AG_ObjectLock(tl);
	if (it->color != NULL) {
		free(it->color);
	}
	if (c != NULL) {
		it->color = Malloc(sizeof(AG_Color));
		memcpy(it->color, c, sizeof(AG_Color));
	} else {
		it->color = NULL;
	}
	AG_ObjectUnlock(tl);
}

/* Set an alternate, per-item font. */
void
AG_TlistSetFont(AG_Tlist *tl, AG_TlistItem *it, AG_Font *font)
{
	AG_ObjectLock(tl);
	if (it->font != NULL && it->font != agDefaultFont) {
		AG_UnusedFont(it->font);
	}
	if (font != NULL) {
		font->nRefs++;
		it->font = font;
	} else {
		it->font = NULL;
	}
	AG_ObjectUnlock(tl);
}

/* Select an item based on its pointer value. */
AG_TlistItem *
AG_TlistSelectPtr(AG_Tlist *tl, void *p)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
#ifdef AG_TIMERS
	UpdatePolled(tl);
#endif
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

	AG_ObjectLock(tl);
#ifdef AG_TIMERS
	UpdatePolled(tl);
#endif
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

/* Set the selection flag on an item. */
void
AG_TlistSelect(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_ObjectLock(tl);
	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	SelectItem(tl, it);
	AG_ObjectUnlock(tl);
}

/* Clear the selection flag on an item. */
void
AG_TlistDeselect(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_ObjectLock(tl);
	DeselectItem(tl, it);
	AG_ObjectUnlock(tl);
}

/* Set the selection flag on all items. */
void
AG_TlistSelectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	TAILQ_FOREACH(it, &tl->items, items) {
		SelectItem(tl, it);
	}
	AG_ObjectUnlock(tl);
}

/* Unset the selection flag on all items. */
void
AG_TlistDeselectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	TAILQ_FOREACH(it, &tl->items, items) {
		DeselectItem(tl, it);
	}
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
	if (tp->panel != NULL) {
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
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_TlistItem *ti;
	const int tind = tl->rOffs + y/tl->item_h + 1;
	
	if (!AG_WidgetIsFocused(tl))
		AG_WidgetFocus(tl);
	
	/* XXX use array */
	if ((ti = AG_TlistFindByIndex(tl, tind)) == NULL)
		return;
	
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
		if (tl->rOffs > (tl->nitems - tl->nvisitems)) {
			tl->rOffs = MAX(0, tl->nitems - tl->nvisitems);
		}
		AG_Redraw(tl);
		break;
	case AG_MOUSE_LEFT:
		/* Expand the children if the user clicked on the [+] sign. */
		if (ti->flags & AG_TLIST_HAS_CHILDREN) {
			if (x >= ti->depth*tl->icon_w &&
			    x <= (ti->depth+1)*tl->icon_w) {
				if (ti->flags & AG_TLIST_ITEM_EXPANDED) {
					ti->flags &= ~AG_TLIST_ITEM_EXPANDED;
				} else {
					ti->flags |=  AG_TLIST_ITEM_EXPANDED;
				}
				tl->flags |= AG_TLIST_REFRESH;
				AG_Redraw(tl);
				return;
			}
		}

		if (ti->flags & AG_TLIST_NO_SELECT) {
			return;
		}
		/*
		 * Handle range selections.
		 */
		if ((tl->flags & AG_TLIST_MULTI) &&
		    (AG_GetModState(tl) & AG_KEYMOD_SHIFT)) {
			AG_TlistItem *oitem;
			int oind = -1, i = 0, nitems = 0;

			TAILQ_FOREACH(oitem, &tl->items, items) {
				if (oitem->selected) {
					oind = i;
				}
				i++;
				nitems++;
			}
			if (oind == -1) {
				return;
			}
			if (oind < tind) {			  /* Forward */
				i = 0;
				TAILQ_FOREACH(oitem, &tl->items, items) {
					if (i == tind)
						break;
					if (i > oind) {
						SelectItem(tl, oitem);
					}
					i++;
				}
			} else if (oind >= tind) {		  /* Backward */
				i = nitems;
				TAILQ_FOREACH_REVERSE(oitem, &tl->items,
				    ag_tlist_itemq, items) {
					if (i <= oind)
						SelectItem(tl, oitem);
					if (i == tind)
						break;
					i--;
				}
			}
			break;
		}
		/*
		 * Handle single selections.
		 */
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
#ifdef AG_TIMERS
		/* Handle double clicks. */
		/* XXX compare the args as well as p1 */
		if (tl->dblClicked != NULL && tl->dblClicked == ti->p1) {
			AG_DelTimer(tl, &tl->dblClickTo);
			if (tl->dblClickEv != NULL) {
				AG_PostEventByPtr(NULL, tl, tl->dblClickEv,
				    "%p", ti);
			}
			AG_PostEvent(NULL, tl, "tlist-dblclick", "%p", ti);
			tl->dblClicked = NULL;
		} else {
			tl->dblClicked = ti->p1;
			AG_AddTimer(tl, &tl->dblClickTo, agMouseDblclickDelay,
			    DoubleClickTimeout, NULL);
		}
#endif
		break;
	case AG_MOUSE_RIGHT:
		if (ti->flags & AG_TLIST_NO_POPUP) {
			return;
		}
		if (tl->popupEv != NULL) {
			AG_PostEventByPtr(NULL, tl, tl->popupEv, NULL);
		} else if (ti->cat != NULL) {
			AG_TlistPopup *tp;
	
			if (!(tl->flags &
			    (AG_TLIST_MULTITOGGLE|AG_TLIST_MULTI)) ||
			    !(AG_GetModState(tl) & (AG_KEYMOD_CTRL|AG_KEYMOD_SHIFT))) {
				AG_TlistDeselectAll(tl);
				SelectItem(tl, ti);
			}
			TAILQ_FOREACH(tp, &tl->popups, popups) {
				if (strcmp(tp->iclass, ti->cat) == 0)
					break;
			}
			if (tp != NULL) {
				PopupMenu(tl, tp, x,y);
				return;
			}
		}
		break;
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	int keysym = AG_INT(1);
	void *ti;

	switch (keysym) {
	case AG_KEY_UP:
		DecrementSelection(tl, 1);
#ifdef AG_TIMERS
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", -1);
#endif
		break;
	case AG_KEY_DOWN:
		IncrementSelection(tl, 1);
#ifdef AG_TIMERS
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", +1);
#endif
		break;
	case AG_KEY_PAGEUP:
		DecrementSelection(tl, agPageIncrement);
#ifdef AG_TIMERS
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", -agPageIncrement);
#endif
		break;
	case AG_KEY_PAGEDOWN:
		IncrementSelection(tl, agPageIncrement);
#ifdef AG_TIMERS
		AG_AddTimer(tl, &tl->moveTo, agKbdDelay, MoveTimeout, "%i", +agPageIncrement);
#endif
		break;
	case AG_KEY_HOME:
		AG_TlistScrollToStart(tl);
		break;
	case AG_KEY_END:
		AG_TlistScrollToEnd(tl);
		break;
	case AG_KEY_RETURN:
		if ((ti = AG_TlistSelectedItemPtr(tl)) != NULL) {
			AG_PostEvent(NULL, tl, "tlist-return", "%p", ti);
		}
		break;
	}
	tl->lastKeyDown = keysym;
}

#ifdef AG_TIMERS
static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	int keysym = AG_INT(1);

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
#endif /* AG_TIMERS */

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
	tl->wSpace = 4;
	tl->icon_w = tl->item_h + 1;
	tl->wRow = 0;
	tl->rOffs = 0;
	tl->dblClicked = NULL;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	tl->nitems = 0;
	tl->nvisitems = 0;
	TAILQ_INIT(&tl->popups);
	tl->compare_fn = AG_TlistComparePtrs;
	tl->popupEv = NULL;
	tl->changedEv = NULL;
	tl->dblClickEv = NULL;
	tl->wheelTicks = 0;
	tl->lastKeyDown = AG_KEY_NONE;
#ifdef AG_TIMERS
	AG_InitTimer(&tl->moveTo, "move", 0);
	AG_InitTimer(&tl->refreshTo, "refresh", 0);
	AG_InitTimer(&tl->dblClickTo, "dblClick", 0);
#endif
	tl->sbar = AG_ScrollbarNew(tl, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_SetInt(tl->sbar, "min", 0);
	AG_BindInt(tl->sbar, "max", &tl->nitems);
	AG_BindInt(tl->sbar, "visible", &tl->nvisitems);
	AG_BindInt(tl->sbar, "value", &tl->rOffs);
	AG_WidgetSetFocusable(tl->sbar, 0);
	
	AG_SetInt(tl, "line-scroll-amount", 5);

	AG_AddEvent(tl, "font-changed", OnFontChange, NULL);
	AG_SetEvent(tl, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tl, "key-down", KeyDown, NULL);
#ifdef AG_TIMERS
	AG_AddEvent(tl, "widget-shown", OnShow, NULL);
	AG_AddEvent(tl, "widget-hidden", OnFocusLoss, NULL);
	AG_SetEvent(tl, "widget-lostfocus", OnFocusLoss, NULL);
	AG_SetEvent(tl, "key-up", KeyUp, NULL);
#endif
	AG_BindPointer(tl, "selected", &tl->selected);
#if 0
	AG_BindInt(tl, "nitems", &tl->nitems);
	AG_BindInt(tl, "nvisitems", &tl->nvisitems);
#endif
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
	AG_Surface *sScaled;

	AG_ObjectLock(tl);
	tl->item_h = ih;
	TAILQ_FOREACH(it, &tl->items, items) {		/* Rescale icons */
		if (it->icon == -1) {
			continue;
		}
		if ((sScaled = AG_SurfaceScale(it->iconsrc,
		    tl->item_h, tl->item_h, 0)) == NULL) {
			AG_FatalError(NULL);
		}
		AG_WidgetReplaceSurface(tl, it->icon, sScaled);
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

/* Set the width to use for item icons. */
void
AG_TlistSetIconWidth(AG_Tlist *tl, int iw)
{
	AG_TlistItem *it;
	AG_Surface *sScaled;

	AG_ObjectLock(tl);
	tl->icon_w = iw;
	TAILQ_FOREACH(it, &tl->items, items) {		/* Rescale icons */
		if (it->icon == -1) {
			continue;
		}
		if ((sScaled = AG_SurfaceScale(it->iconsrc,
		    tl->item_h, tl->item_h, 0)) == NULL) {
			AG_FatalError(NULL);
		}
		AG_WidgetReplaceSurface(tl, it->icon, sScaled);
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

void
AG_TlistSetDblClickFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->dblClickEv = AG_SetEvent(tl, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(tl->dblClickEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetPopupFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->popupEv = AG_SetEvent(tl, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(tl->popupEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetChangedFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->changedEv = AG_SetEvent(tl, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(tl->changedEv, fmt);
	AG_ObjectUnlock(tl);
}

/* Create a new popup menu for items of the given class. */
AG_MenuItem *
AG_TlistSetPopup(AG_Tlist *tl, const char *iclass)
{
	AG_TlistPopup *tp;

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
	tl->rOffs = 0;
	AG_Redraw(tl);
}

/* Scroll to the end of the list. */
void
AG_TlistScrollToEnd(AG_Tlist *tl)
{
	tl->rOffs = MAX(0, tl->nitems - tl->nvisitems);
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
int
AG_TlistSort(AG_Tlist *tl)
{
	AG_TlistItem *it, **items;
	Uint i = 0;

	if ((items = TryMalloc(tl->nitems*sizeof(AG_TlistItem *))) == NULL) {
		return (-1);
	}
	TAILQ_FOREACH(it, &tl->items, items) {
		items[i++] = it;
	}
	qsort(items, tl->nitems, sizeof(AG_TlistItem *), CompareText);
	TAILQ_INIT(&tl->items);
	for (i = 0; i < tl->nitems; i++) {
		TAILQ_INSERT_TAIL(&tl->items, items[i], items);
	}
	free(items);
	AG_Redraw(tl);
	return (0);
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
		AG_GenericMismatch("AG_TLIST_ITEM_PTR(idx)");
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
		AG_GenericMismatch("AG_TLIST_ITEM_PTR(tag)");
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
