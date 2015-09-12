/*
 * Copyright (c) 2002-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

static void MouseButtonDown(AG_Event *);
static void KeyDown(AG_Event *);
static void KeyUp(AG_Event *);

static void FreeItem(AG_Tlist *, AG_TlistItem *);
static void SelectItem(AG_Tlist *, AG_TlistItem *);
static void DeselectItem(AG_Tlist *, AG_TlistItem *);
static void UpdateItemIcon(AG_Tlist *, AG_TlistItem *, AG_Surface *);

#ifndef AG_TLIST_PADDING
#define AG_TLIST_PADDING 2	/* Label padding (pixels) */
#endif

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

static __inline__ void
UpdatePolled(AG_Tlist *tl)
{
	if ((tl->flags & AG_TLIST_POLL) &&
	    (tl->flags & AG_TLIST_REFRESH)) {
		tl->flags &= ~(AG_TLIST_REFRESH);
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
}

static int
SelectionVisible(AG_Tlist *tl)
{
	AG_TlistItem *it;
	int y = 0, i = 0;

	UpdatePolled(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		if (i++ < tl->rOffs)
			continue;
		if (y > HEIGHT(tl) - tl->item_h)
			break;

		if (it->selected) {
			return (1);
		}
		y += tl->item_h;
	}
	return (0);
}

static void
ScrollToSelection(AG_Tlist *tl)
{
	AG_TlistItem *it;
	int m = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected) {
			m++;
			continue;
		}
		tl->rOffs = (tl->rOffs > m) ? m :
		    MAX(0, m - tl->nvisitems + 1);
		AG_Redraw(tl);
		return;
	}
}

static void
DecrementSelection(AG_Tlist *tl, int inc)
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
IncrementSelection(AG_Tlist *tl, int inc)
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

static Uint32
DoubleClickTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	tl->dblClicked = NULL;
	return (0);
}

static void
OnFocusLoss(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	AG_DelTimer(tl, &tl->moveTo);
	AG_DelTimer(tl, &tl->dblClickTo);
}

/* Timer for updates in AG_TLIST_POLL mode. */
static Uint32
PollRefreshTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	tl->flags |= AG_TLIST_REFRESH;
	AG_Redraw(tl);
	return (to->ival);
}

static void
OnFontChange(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
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

static void
OnShow(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	if (tl->flags & AG_TLIST_POLL) {
		tl->flags |= AG_TLIST_REFRESH;
		AG_AddTimer(tl, &tl->refreshTo, 125, PollRefreshTimeout, NULL);
	}
}

/* Timer for moving keyboard selection. */
static Uint32
MoveTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int incr = AG_INT(1);

	if (incr < 0) {
		DecrementSelection(tl, -incr);
	} else {
		IncrementSelection(tl, incr);
	}
	return (agKbdRepeat);
}

static void
Init(void *obj)
{
	AG_Tlist *tl = obj;

	WIDGET(tl)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_USE_TEXT;

	tl->flags = 0;
	tl->selected = NULL;
	tl->wSpace = 4;
	tl->item_h = agTextFontHeight + AG_TLIST_PADDING;
	tl->icon_w = tl->item_h + 1;
	tl->dblClicked = NULL;
	tl->nitems = 0;
	tl->nvisitems = 0;
	tl->compare_fn = AG_TlistComparePtrs;
	tl->wHint = 0;
	tl->hHint = tl->item_h + 2;
	tl->popupEv = NULL;
	tl->changedEv = NULL;
	tl->dblClickEv = NULL;
	tl->wheelTicks = 0;
	tl->r = AG_RECT(0,0,0,0);
	tl->rOffs = 0;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	TAILQ_INIT(&tl->popups);

	AG_InitTimer(&tl->moveTo, "move", 0);
	AG_InitTimer(&tl->refreshTo, "refresh", 0);
	AG_InitTimer(&tl->dblClickTo, "dblClick", 0);

	tl->sbar = AG_ScrollbarNew(tl, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_SetInt(tl->sbar, "min", 0);
	AG_BindInt(tl->sbar, "max", &tl->nitems);
	AG_BindInt(tl->sbar, "visible", &tl->nvisitems);
	AG_BindInt(tl->sbar, "value", &tl->rOffs);
	AG_WidgetSetFocusable(tl->sbar, 0);

	AG_AddEvent(tl, "font-changed", OnFontChange, NULL);
	AG_AddEvent(tl, "widget-shown", OnShow, NULL);
	AG_AddEvent(tl, "widget-hidden", OnFocusLoss, NULL);
	AG_SetEvent(tl, "widget-lostfocus", OnFocusLoss, NULL);
	AG_SetEvent(tl, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tl, "key-down", KeyDown, NULL);
	AG_SetEvent(tl, "key-up", KeyUp, NULL);
	
	AG_BindPointer(tl, "selected", &tl->selected);

#ifdef AG_DEBUG
	AG_BindInt(tl, "nitems", &tl->nitems);
	AG_BindInt(tl, "nvisitems", &tl->nvisitems);
#endif /* AG_DEBUG */
}

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
	UpdatePolled(tl);
	tl->wHint = 0;
	AG_TLIST_FOREACH(it, tl) {
		AG_TextSize(it->text, &w, NULL);
		if (w > tl->wHint) { tl->wHint = w; }
	}
	tl->wHint += tl->icon_w*4;
	tl->hHint = (tl->item_h+2)*nitems;
	AG_ObjectUnlock(tl);
}

static void
Destroy(void *p)
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
		Free(tp);
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Tlist *tl = obj;
	AG_SizeReq rBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);
	r->w = tl->icon_w + tl->wSpace*2 + tl->wHint + rBar.w;
	r->h = tl->hHint;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Tlist *tl = obj;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);
	if (a->w < rBar.w*2) {
		rBar.w = MAX(0, a->w/2);
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
DrawSubnodeIndicator(void *wid, AG_Rect r, int isExpanded)
{
	AG_Color C;

	AG_DrawRectBlended(wid,
	    AG_RECT(r.x-1, r.y, r.w+2, r.h),
	    AG_ColorRGBA(0,0,0,64),
	    AG_ALPHA_SRC);

	C = AG_ColorRGBA(255,255,255,100);
	if (isExpanded) {
		AG_DrawMinus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    C, AG_ALPHA_SRC);
	} else {
		AG_DrawPlus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    C, AG_ALPHA_SRC);
	}
}

static void
Draw(void *obj)
{
	AG_Tlist *tl = obj;
	AG_TlistItem *it;
	int y = 0, i = 0, selSeen = 0, selPos = 1;

	AG_DrawBox(tl, tl->r, -1, WCOLOR(tl,AG_COLOR));
	AG_WidgetDraw(tl->sbar);
	AG_PushClipRect(tl, tl->r);

	UpdatePolled(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		int x = 2 + it->depth*tl->icon_w;

		if (i++ < tl->rOffs) {
			if (it->selected) {
				selPos = -1;
			}
			continue;
		}
		if (y > HEIGHT(tl) - tl->item_h)
			break;

		if (it->selected) {
		    	AG_Rect rSel;
			rSel.x = x + tl->icon_w + 2;
			rSel.y = y;
			rSel.w = tl->wRow - x - tl->icon_w - 3;
			rSel.h = tl->item_h + 1;
			AG_DrawRect(tl, rSel, WCOLOR_SEL(tl,AG_COLOR));
			selSeen = 1;
		}
		if (it->iconsrc != NULL) {
			if (it->icon == -1) {
				AG_Surface *scaled = NULL;

				if (AG_ScaleSurface(it->iconsrc,
				    tl->icon_w, tl->item_h, &scaled) == -1) {
					AG_FatalError(NULL);
				}
				it->icon = AG_WidgetMapSurface(tl, scaled);
			}
			AG_WidgetBlitSurface(tl, it->icon, x, y);
		}
		if (it->flags & AG_TLIST_HAS_CHILDREN) {
			DrawSubnodeIndicator(tl,
			    AG_RECT(x,
			            y,
				    tl->icon_w,
				    tl->item_h),
			    (it->flags & AG_TLIST_VISIBLE_CHILDREN));
		}
		if (it->label == -1) {
			AG_TextColor(it->selected ?
			             WCOLOR_SEL(tl,AG_TEXT_COLOR) :
				     WCOLOR(tl,AG_TEXT_COLOR));
			it->label = AG_WidgetMapSurface(tl,
			    AG_TextRender(it->text));
		}

		if ((y + tl->item_h) < HEIGHT(tl)-1)
			AG_DrawLineH(tl, 1, tl->wRow-2, (y + tl->item_h),
			    WCOLOR(tl,AG_LINE_COLOR));

		AG_WidgetBlitSurface(tl, it->label,
		    x + tl->icon_w + tl->wSpace,
		    y + AG_TLIST_PADDING);
		y += tl->item_h;
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
FreeItem(AG_Tlist *tl, AG_TlistItem *it)
{
	if (it->label != -1) {
		AG_WidgetUnmapSurface(tl, it->label);
	}
	if (it->flags & AG_TLIST_DYNICON && it->iconsrc != NULL) {
		AG_SurfaceFree(it->iconsrc);
	}
	if (it->icon != -1) {
		AG_WidgetUnmapSurface(tl, it->icon);
	}
	Free(it);
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
AG_TlistClear(AG_Tlist *tl)
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
AG_TlistCompareStrings(const AG_TlistItem *it1,
    const AG_TlistItem *it2)
{
	return (it1->text != NULL && it2->text != NULL &&
	        strcmp(it1->text, it2->text) == 0);
}

/* Generic pointer compare routine. */
int
AG_TlistComparePtrs(const AG_TlistItem *it1, const AG_TlistItem *it2)
{
	return (it1->p1 == it2->p1);
}

/* Generic pointer+class compare routine. */
int
AG_TlistComparePtrsAndClasses(const AG_TlistItem *it1,
    const AG_TlistItem *it2)
{
	return ((it1->p1 == it2->p1) &&
	        (it1->cat != NULL && it2->cat!= NULL &&
		 (strcmp(it1->cat, it2->cat) == 0)));
}

/* Set an alternate compare function for items. */
void
AG_TlistSetCompareFn(AG_Tlist *tl,
    int (*fn)(const AG_TlistItem *, const AG_TlistItem *))
{
	AG_ObjectLock(tl);
	tl->compare_fn = fn;
	AG_ObjectUnlock(tl);
}

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

/* Restore previous item selection state. */
void
AG_TlistRestore(AG_Tlist *tl)
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
			if (sit->flags & AG_TLIST_VISIBLE_CHILDREN) {
				cit->flags |= AG_TLIST_VISIBLE_CHILDREN;
			} else {
				cit->flags &= ~(AG_TLIST_VISIBLE_CHILDREN);
			}
		}
		FreeItem(tl, sit);
	}
	TAILQ_INIT(&tl->selitems);

	AG_ObjectUnlock(tl);
}

/*
 * Allocate a new tlist item.
 * XXX allocate from a pool, especially for polled items.
 */
static __inline__ AG_TlistItem *
AllocItem(AG_Tlist *tl, AG_Surface *iconsrc)
{
	AG_TlistItem *it;

	it = Malloc(sizeof(AG_TlistItem));
	it->selected = 0;
	it->cat = "";
	it->depth = 0;
	it->flags = 0;
	it->icon = -1;
	it->label = -1;
	UpdateItemIcon(tl, it, iconsrc);
	return (it);
}

/* The Tlist must be locked. */
static __inline__ void
InsertItem(AG_Tlist *tl, AG_TlistItem *it, int ins_head)
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
AG_TlistAddPtr(AG_Tlist *tl, AG_Surface *iconsrc, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AllocItem(tl, iconsrc);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the tail of the list (format string) */
AG_TlistItem *
AG_TlistAdd(AG_Tlist *tl, AG_Surface *iconsrc, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;
	
	AG_ObjectLock(tl);
	it = AllocItem(tl, iconsrc);
	it->p1 = NULL;
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the tail of the list (plain string) */
AG_TlistItem *
AG_TlistAddS(AG_Tlist *tl, AG_Surface *iconsrc, const char *text)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AllocItem(tl, iconsrc);
	it->p1 = NULL;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 0);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (format string) */
AG_TlistItem *
AG_TlistAddHead(AG_Tlist *tl, AG_Surface *iconsrc, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;
	
	AG_ObjectLock(tl);
	it = AllocItem(tl, iconsrc);
	it->p1 = NULL;
	va_start(args, fmt);
	Vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (plain string) */
AG_TlistItem *
AG_TlistAddHeadS(AG_Tlist *tl, AG_Surface *iconsrc, const char *text)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AllocItem(tl, iconsrc);
	it->p1 = NULL;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Add an item to the head of the list (user pointer) */
AG_TlistItem *
AG_TlistAddPtrHead(AG_Tlist *tl, AG_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	it = AllocItem(tl, icon);
	it->p1 = p1;
	Strlcpy(it->text, text, sizeof(it->text));
	InsertItem(tl, it, 1);
	AG_ObjectUnlock(tl);
	return (it);
}

/* Select an item based on its pointer value. */
AG_TlistItem *
AG_TlistSelectPtr(AG_Tlist *tl, void *p)
{
	AG_TlistItem *it;

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

/* The Tlist must be locked. */
static void
SelectItem(AG_Tlist *tl, AG_TlistItem *it)
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

/* The Tlist must be locked. */
static void
DeselectItem(AG_Tlist *tl, AG_TlistItem *it)
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
PopupMenu(AG_Tlist *tl, AG_TlistPopup *tp, int x, int y)
{
	AG_Menu *m = tp->menu;
	
#if 0
	if (AG_ParentWindow(tl) == NULL)
		AG_FatalError("AG_Tlist: %s is unattached", OBJECT(tl)->name);
#endif
	if (tp->panel != NULL) {
		AG_MenuCollapse(tp->item);
		tp->panel = NULL;
	}
	m->itemSel = tp->item;
	tp->panel = AG_MenuExpand(tl, tp->item, x+4, y+4);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_TlistItem *ti;
	int tind;

	tind = tl->rOffs + y/tl->item_h + 1;

	/* XXX use array */
	if ((ti = AG_TlistFindByIndex(tl, tind)) == NULL)
		return;
	
	if (!AG_WidgetIsFocused(tl))
		AG_WidgetFocus(tl);
	
	switch (button) {
	case AG_MOUSE_WHEELUP:
		tl->rOffs -= AG_WidgetScrollDelta(&tl->wheelTicks);
		if (tl->rOffs < 0) {
			tl->rOffs = 0;
		}
		AG_Redraw(tl);
		break;
	case AG_MOUSE_WHEELDOWN:
		tl->rOffs += AG_WidgetScrollDelta(&tl->wheelTicks);
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
				if (ti->flags & AG_TLIST_VISIBLE_CHILDREN) {
					ti->flags &= ~AG_TLIST_VISIBLE_CHILDREN;
				} else {
					ti->flags |=  AG_TLIST_VISIBLE_CHILDREN;
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
KeyDown(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int keysym = AG_INT(1);
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
	case AG_KEY_RETURN:
		if ((ti = AG_TlistSelectedItemPtr(tl)) != NULL) {
			AG_PostEvent(NULL, tl, "tlist-return", "%p", ti);
		}
		break;
	}
	tl->lastKeyDown = keysym;
}

static void
KeyUp(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
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

	AG_ObjectLock(tl);
	tl->item_h = ih;
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != -1) {
			AG_Surface *scaled = NULL;

			if (AG_ScaleSurface(it->iconsrc,
			    tl->item_h, tl->item_h, &scaled) == -1) {
				AG_FatalError(NULL);
			}
			AG_WidgetReplaceSurface(tl, it->icon, scaled);
		}
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

/* Set the width to use for item icons. */
void
AG_TlistSetIconWidth(AG_Tlist *tl, int iw)
{
	AG_TlistItem *it;

	AG_ObjectLock(tl);
	tl->icon_w = iw;
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != -1) {
			AG_Surface *scaled = NULL;

			if (AG_ScaleSurface(it->iconsrc,
			    tl->item_h, tl->item_h, &scaled) == -1) {
				AG_FatalError(NULL);
			}
			AG_WidgetReplaceSurface(tl, it->icon, scaled);
		}
	}
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

/* Update the icon associated with an item. The Tlist must be locked. */
static void
UpdateItemIcon(AG_Tlist *tl, AG_TlistItem *it, AG_Surface *iconsrc)
{
	if (it->flags & AG_TLIST_DYNICON) {
		if (it->iconsrc != NULL) {
			AG_SurfaceFree(it->iconsrc);
		}
		if (iconsrc != NULL) {
			it->iconsrc = AG_SurfaceDup(iconsrc);
		} else {
			it->iconsrc = NULL;
		}
	} else {
		it->iconsrc = iconsrc;
	}

	if (it->icon != -1) {
		AG_WidgetUnmapSurface(tl, it->icon);
		it->icon = -1;
	}
}

void
AG_TlistSetIcon(AG_Tlist *tl, AG_TlistItem *it, AG_Surface *iconsrc)
{
	AG_ObjectLock(tl);
	it->flags |= AG_TLIST_DYNICON;
	UpdateItemIcon(tl, it, iconsrc);
	AG_ObjectUnlock(tl);
	AG_Redraw(tl);
}

void
AG_TlistSetDblClickFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->dblClickEv = AG_SetVoidFn(tl, fn, NULL);
	AG_EVENT_GET_ARGS(tl->dblClickEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetPopupFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->popupEv = AG_SetVoidFn(tl, fn, NULL);
	AG_EVENT_GET_ARGS(tl->popupEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetChangedFn(AG_Tlist *tl, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->changedEv = AG_SetVoidFn(tl, fn, NULL);
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
CompareText(const void *p1, const void *p2)
{
	const AG_TlistItem *it1 = *(const AG_TlistItem **)p1;
	const AG_TlistItem *it2 = *(const AG_TlistItem **)p2;

	return strcoll(it1->text, it2->text);
}

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

AG_WidgetClass agTlistClass = {
	{
		"Agar(Widget:Tlist)",
		sizeof(AG_Tlist),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
