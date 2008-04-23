/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <core/config.h>

#include "tlist.h"
#include "primitive.h"

#include <string.h>
#include <stdarg.h>

static void MouseButtonDown(AG_Event *);
static void KeyDown(AG_Event *);
static void KeyUp(AG_Event *);

static void FreeItem(AG_Tlist *, AG_TlistItem *);
static void SelectItem(AG_Tlist *, AG_TlistItem *);
static void DeselectItem(AG_Tlist *, AG_TlistItem *);
static void PopupMenu(AG_Tlist *, AG_TlistPopup *);
static void UpdateItemIcon(AG_Tlist *, AG_TlistItem *, AG_Surface *);
static void UpdateListScrollbar(AG_Tlist *);
static void ScrollbarChanged(AG_Event *);

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
	return (tl);
}

static int
SelectionVisible(AG_Tlist *tl)
{
	AG_TlistItem *it;
	int y = 0, i = 0;
	int offset;

	if (tl->flags & AG_TLIST_POLL) {
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
	offset = AG_WidgetInt(tl->sbar, "value");

	TAILQ_FOREACH(it, &tl->items, items) {
		if (i++ < offset)
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
	int offset = AG_WidgetInt(tl->sbar, "value");

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected) {
			m++;
			continue;
		}
		if (offset > m) {
			AG_WidgetSetInt(tl->sbar, "value", m);
		} else {
			offset = m - tl->nvisitems + 1;
			if (offset < 0) { offset = 0; }
			AG_WidgetSetInt(tl->sbar, "value", offset);
		}
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

static void
DoubleClickTimeout(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	tl->dblclicked = NULL;
}

static void
LostFocus(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_DelTimeout(tl, &tl->incTo);
	AG_DelTimeout(tl, &tl->decTo);
	AG_CancelEvent(tl, "dblclick-expire");
}

static Uint32
DecrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Tlist *tl = obj;
	Uint8 *ks;
	int numkeys;

	ks = SDL_GetKeyState(&numkeys);
	DecrementSelection(tl, ks[SDLK_PAGEUP] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static Uint32
IncrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Tlist *tl = obj;
	Uint8 *ks;
	int numkeys;

	ks = SDL_GetKeyState(&numkeys);
	IncrementSelection(tl, ks[SDLK_PAGEDOWN] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static void
Init(void *obj)
{
	AG_Tlist *tl = obj;

	WIDGET(tl)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	AG_WidgetBind(tl, "selected", AG_WIDGET_POINTER, &tl->selected);

	tl->flags = 0;
	tl->selected = NULL;
	tl->wSpace = 4;
	tl->item_h = agTextFontHeight+2;
	tl->icon_w = tl->item_h - 4;
	tl->dblclicked = NULL;
	tl->nitems = 0;
	tl->nvisitems = 0;
	tl->sbar = AG_ScrollbarNew(tl, AG_SCROLLBAR_VERT, 0);
	tl->compare_fn = AG_TlistComparePtrs;
	tl->wHint = 0;
	tl->hHint = tl->item_h + 2;
	tl->popupEv = NULL;
	tl->changedEv = NULL;
	tl->dblClickEv = NULL;
	tl->wheelTicks = 0;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	TAILQ_INIT(&tl->popups);

	AG_SetEvent(tl->sbar, "scrollbar-changed", ScrollbarChanged, "%p", tl);
	AG_SetEvent(tl, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(tl, "window-keydown", KeyDown, NULL);
	AG_SetEvent(tl, "window-keyup", KeyUp, NULL);
	AG_SetEvent(tl, "dblclick-expire", DoubleClickTimeout, NULL);
	AG_SetEvent(tl, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(tl, "widget-hidden", LostFocus, NULL);
	
	AG_SetTimeout(&tl->decTo, DecrementTimeout, NULL, 0);
	AG_SetTimeout(&tl->incTo, IncrementTimeout, NULL, 0);
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
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Tlist *tl = p;
	AG_SizeReq rBar;

	AG_WidgetSizeReq(tl->sbar, &rBar);
	r->w = tl->icon_w + tl->wSpace*2 + tl->wHint + rBar.w;
	r->h = tl->hHint;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Tlist *tl = p;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	
	AG_WidgetSizeReq(tl->sbar, &rBar);
	if (rBar.w > a->w) {
		return (-1);
	}
	aBar.w = rBar.w;
	aBar.h = a->h;
	aBar.x = a->w - rBar.w;
	aBar.y = 0;
	AG_WidgetSizeAlloc(tl->sbar, &aBar);

	if (a->w < rBar.w || a->h < tl->sbar->wButton*2) {
		WIDGET(tl->sbar)->flags |= AG_WIDGET_HIDE;
	} else {
		WIDGET(tl->sbar)->flags &= ~(AG_WIDGET_HIDE);
	}
	UpdateListScrollbar(tl);
	return (0);
}

static void
Draw(void *p)
{
	AG_Tlist *tl = p;
	AG_TlistItem *it;
	int y = 0, i = 0;
	int offset;

	STYLE(tl)->ListBackground(tl,
	    AG_RECT(0, 0, WIDTH(tl), HEIGHT(tl)));

	if (tl->flags & AG_TLIST_POLL) {
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
	offset = AG_WidgetInt(tl->sbar, "value");
	TAILQ_FOREACH(it, &tl->items, items) {
		int x = 2 + it->depth*tl->icon_w;

		if (i++ < offset)
			continue;
		if (y > HEIGHT(tl) - tl->item_h)
			break;

		STYLE(tl)->ListItemBackground(tl,
		    AG_RECT(x+tl->icon_w+2, y, WIDTH(tl)-x,tl->item_h),
		    it->selected);

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
			STYLE(tl)->TreeSubnodeIndicator(tl,
			    AG_RECT(x, y, tl->icon_w, tl->item_h),
			    (it->flags & AG_TLIST_VISIBLE_CHILDREN));
		}
		if (it->label == -1) {
			AG_TextColor(TLIST_TXT_COLOR);
			it->label = AG_WidgetMapSurface(tl,
			    AG_TextRender(it->text));
		}
		AG_WidgetBlitSurface(tl, it->label,
		    x + tl->icon_w + tl->wSpace,
		    y + tl->item_h/2 - WSURFACE(tl,it->label)->h/2);

		y += tl->item_h;
		if (y < HEIGHT(tl)-1)
			AG_DrawLineH(tl, 0, WIDTH(tl), y,
			    AG_COLOR(TLIST_LINE_COLOR));
	}
}

/*
 * Adjust the scrollbar offset according to the number of visible items.
 * Tlist must be locked.
 */
static void
UpdateListScrollbar(AG_Tlist *tl)
{
	AG_WidgetBinding *maxb, *offsetb;
	int *max, *offset;
	int noffset;
	
	tl->nvisitems = HEIGHT(tl)/tl->item_h;

	maxb = AG_WidgetGetBinding(tl->sbar, "max", &max);
	offsetb = AG_WidgetGetBinding(tl->sbar, "value", &offset);
	noffset = *offset;

	*max = tl->nitems - tl->nvisitems;

	if (noffset > *max)
		noffset = *max;
	if (noffset < 0)
		noffset = 0;

	if (*offset != noffset) {
		*offset = noffset;
		AG_WidgetBindingChanged(offsetb);
	}

	if (tl->nitems > 0 && tl->nvisitems > 0 &&
	    tl->nvisitems < tl->nitems) {
		AG_ScrollbarSetBarSize(tl->sbar,
		    tl->nvisitems*(HEIGHT(tl->sbar) - tl->sbar->wButton*2) /
		    tl->nitems);
	} else {
		AG_ScrollbarSetBarSize(tl->sbar, -1);		/* Full range */
	}

	AG_WidgetUnlockBinding(offsetb);
	AG_WidgetUnlockBinding(maxb);
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

void
AG_TlistDel(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_WidgetBinding *offsetb;
	int *offset;
	int nitems;

	AG_ObjectLock(tl);
	TAILQ_REMOVE(&tl->items, it, items);
	nitems = --tl->nitems;
	FreeItem(tl, it);

	/* Update the scrollbar range and offset accordingly. */
	AG_WidgetSetInt(tl->sbar, "max", nitems);
	offsetb = AG_WidgetGetBinding(tl->sbar, "value", &offset);
	if (*offset > nitems) {
		*offset = nitems;
	} else if (nitems > 0 && *offset < nitems) {		/* XXX ugly */
		*offset = 0;
	}
	AG_WidgetUnlockBinding(offsetb);
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
	
	/* Preserve the offset value, for polling. */
	AG_WidgetSetInt(tl->sbar, "max", 0);

	AG_ObjectUnlock(tl);
}

int
AG_TlistCompareStrings(const AG_TlistItem *it1,
    const AG_TlistItem *it2)
{
	return (it1->text != NULL && it2->text != NULL &&
	        strcmp(it1->text, it2->text) == 0);
}

int
AG_TlistComparePtrs(const AG_TlistItem *it1, const AG_TlistItem *it2)
{
	if (it1->argc != it2->argc ||
	   (it1->argc > 0 &&
	    memcmp(it1->argv, it2->argv, it1->argc*sizeof(union evarg)) != 0)) {
		return (0);
	}
	return (it1->p1 == it2->p1);
}

int
AG_TlistComparePtrsAndClasses(const AG_TlistItem *it1,
    const AG_TlistItem *it2)
{
	return ((it1->p1 == it2->p1) &&
	        (it1->cat != NULL && it2->cat!= NULL &&
		 (strcmp(it1->cat, it2->cat) == 0)));
}

void
AG_TlistSetCompareFn(AG_Tlist *tl,
    int (*fn)(const AG_TlistItem *, const AG_TlistItem *))
{
	AG_ObjectLock(tl);
	tl->compare_fn = fn;
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

	UpdateListScrollbar(tl);
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
	it->argc = 0;
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
	AG_WidgetSetInt(tl->sbar, "max", ++tl->nitems);
}

/* Add an item to the list. */
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

void
AG_TlistSetArgs(AG_TlistItem *it, const char *fmt, ...)
{
	va_list ap;
	const char *s = fmt;

	va_start(ap, fmt);
	while (*s != '\0') {
		switch (*s) {
		case 'p':
			it->argv[it->argc++].p = va_arg(ap, void *);
			break;
		case 's':
			it->argv[it->argc++].s = va_arg(ap, char *);
			break;
		case 'i':
		case 'u':
			it->argv[it->argc++].i = va_arg(ap, int);
			break;
		case 'D':
		case 'U':
			it->argv[it->argc++].li = va_arg(ap, long int);
			break;
		case 'f':
		case 'd':
			it->argv[it->argc++].f = va_arg(ap, double);
			break;
		case 'c':
			it->argv[it->argc++].i = va_arg(ap, int);
			break;
		default:
			break;
		}
		s++;
	}
	va_end(ap);
}

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
	if (tl->flags & AG_TLIST_POLL) {
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
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
	if (tl->flags & AG_TLIST_POLL) {
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
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
	AG_WidgetBinding *selectedb;
	void **sel_ptr;

	selectedb = AG_WidgetGetBinding(tl, "selected", &sel_ptr);
	*sel_ptr = it->p1;
	if (!it->selected) {
		it->selected = 1;
		if (tl->changedEv != NULL) {
			AG_PostEvent(NULL, tl, tl->changedEv->name, "%p,%i",
			    it, 1);
		}
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 1);
	}
	AG_PostEvent(NULL, tl, "tlist-selected", "%p", it);
	AG_WidgetBindingChanged(selectedb);
	AG_WidgetUnlockBinding(selectedb);
}

/* The Tlist must be locked. */
static void
DeselectItem(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_WidgetBinding *selectedb;
	void **sel_ptr;

	selectedb = AG_WidgetGetBinding(tl, "selected", &sel_ptr);
	*sel_ptr = NULL;
	if (it->selected) {
		it->selected = 0;
		if (tl->changedEv != NULL) {
			AG_PostEvent(NULL, tl, tl->changedEv->name, "%p,%i",
			    it, 0);
		}
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 0);
	}
	AG_WidgetBindingChanged(selectedb);
	AG_WidgetUnlockBinding(selectedb);
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

	tind = (AG_WidgetInt(tl->sbar, "value") + y/tl->item_h) + 1;

	/* XXX use array */
	if ((ti = AG_TlistFindByIndex(tl, tind)) == NULL)
		return;

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		{
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(tl->sbar, "value", &offs);
			(*offs) -= AG_WidgetScrollDelta(&tl->wheelTicks);
			if (*offs < 0) {
				*offs = 0;
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		{
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(tl->sbar, "value", &offs);
			(*offs) += AG_WidgetScrollDelta(&tl->wheelTicks);
			if (*offs > (tl->nitems - tl->nvisitems)) {
				*offs = tl->nitems - tl->nvisitems;
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_LEFT:
		/* Expand the children if the user clicked on the [+] sign. */
		if (ti->flags & AG_TLIST_HAS_CHILDREN) {
			x -= 7;
			if (x >= ti->depth*tl->icon_w &&
			    x <= (ti->depth+1)*tl->icon_w) {
				if (ti->flags & AG_TLIST_VISIBLE_CHILDREN) {
					ti->flags &= ~AG_TLIST_VISIBLE_CHILDREN;
				} else {
					ti->flags |=  AG_TLIST_VISIBLE_CHILDREN;
				}
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
		    (SDL_GetModState() & KMOD_SHIFT)) {
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
		     (SDL_GetModState() & KMOD_CTRL))) {
			if (ti->selected) {
				DeselectItem(tl, ti);
			} else {
				SelectItem(tl, ti);
			}
			break;
		}

		AG_WidgetFocus(tl);
		AG_TlistDeselectAll(tl);
		SelectItem(tl, ti);

		/* Handle double clicks. */
		/* XXX compare the args as well as p1 */
		if (tl->dblclicked != NULL && tl->dblclicked == ti->p1) {
			AG_CancelEvent(tl, "dblclick-expire");
			if (tl->dblClickEv != NULL) {
				AG_PostEvent(NULL, tl, tl->dblClickEv->name,
				    "%p", ti->p1);
			}
			AG_PostEvent(NULL, tl, "tlist-dblclick", "%p", ti->p1);
			tl->dblclicked = NULL;
		} else {
			tl->dblclicked = ti->p1;
			AG_SchedEvent(NULL, tl, agMouseDblclickDelay,
			    "dblclick-expire", NULL);
		}
		break;
	case SDL_BUTTON_RIGHT:
		if (ti->flags & AG_TLIST_NO_POPUP) {
			return;
		}
		if (tl->popupEv != NULL) {
			AG_PostEvent(NULL, tl, tl->popupEv->name, NULL);
		} else if (ti->cat != NULL) {
			AG_TlistPopup *tp;
	
			AG_WidgetFocus(tl);
		
			if (!(tl->flags &
			    (AG_TLIST_MULTITOGGLE|AG_TLIST_MULTI)) ||
			    !(SDL_GetModState() & (KMOD_CTRL|KMOD_SHIFT))) {
				AG_TlistDeselectAll(tl);
				SelectItem(tl, ti);
			}
			TAILQ_FOREACH(tp, &tl->popups, popups) {
				if (strcmp(tp->iclass, ti->cat) == 0)
					break;
			}
			if (tp != NULL) {
				PopupMenu(tl, tp);
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

	switch (keysym) {
	case SDLK_UP:
		DecrementSelection(tl, 1);
		AG_DelTimeout(tl, &tl->incTo);
		AG_ReplaceTimeout(tl, &tl->decTo, agKbdDelay);
		break;
	case SDLK_DOWN:
		IncrementSelection(tl, 1);
		AG_DelTimeout(tl, &tl->decTo);
		AG_ReplaceTimeout(tl, &tl->incTo, agKbdDelay);
		break;
	case SDLK_PAGEUP:
		DecrementSelection(tl, agPageIncrement);
		AG_DelTimeout(tl, &tl->incTo);
		AG_ReplaceTimeout(tl, &tl->decTo, agKbdDelay);
		break;
	case SDLK_PAGEDOWN:
		IncrementSelection(tl, agPageIncrement);
		AG_DelTimeout(tl, &tl->decTo);
		AG_ReplaceTimeout(tl, &tl->incTo, agKbdDelay);
		break;
	}
}

static void
KeyUp(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_UP:
	case SDLK_PAGEUP:
		AG_DelTimeout(tl, &tl->decTo);
		break;
	case SDLK_DOWN:
	case SDLK_PAGEDOWN:
		AG_DelTimeout(tl, &tl->incTo);
		break;
	}
}

static void
ScrollbarChanged(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);

	AG_ObjectLock(tl);
	UpdateListScrollbar(tl);
	AG_ObjectUnlock(tl);
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
			    tl->item_h-1, tl->item_h-1, &scaled) == -1) {
				AG_FatalError(NULL);
			}
			AG_WidgetReplaceSurface(tl, it->icon, scaled);
		}
	}
	AG_ObjectUnlock(tl);
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
			it->iconsrc = AG_DupSurface(iconsrc);
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
}

void
AG_TlistSetDblClickFn(AG_Tlist *tl, void (*ev)(AG_Event *), const char *fmt,
    ...)
{
	AG_ObjectLock(tl);
	tl->dblClickEv = AG_SetEvent(tl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tl->dblClickEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetPopupFn(AG_Tlist *tl, void (*ev)(AG_Event *), const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->popupEv = AG_SetEvent(tl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tl->popupEv, fmt);
	AG_ObjectUnlock(tl);
}

void
AG_TlistSetChangedFn(AG_Tlist *tl, void (*ev)(AG_Event *), const char *fmt, ...)
{
	AG_ObjectLock(tl);
	tl->changedEv = AG_SetEvent(tl, NULL, ev, NULL);
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

	AG_ObjectLock(tl);
	TAILQ_INSERT_TAIL(&tl->popups, tp, popups);
	AG_ObjectUnlock(tl);
	return (tp->item);
}

/* The Tlist must be locked. */
static void
PopupMenu(AG_Tlist *tl, AG_TlistPopup *tp)
{
	AG_Menu *m = tp->menu;
	int x, y;

#if 0
	if (AG_WidgetParentWindow(tl) == NULL)
		AG_FatalError("AG_Tlist: %s is unattached", OBJECT(tl)->name);
#endif
	AG_MouseGetState(&x, &y);

	if (tp->panel != NULL) {
		AG_MenuCollapse(m, tp->item);
		tp->panel = NULL;
	}
#if 0
	if (m->itemSel != NULL) {
		AG_MenuCollapse(m, m->itemSel);
	}
#endif
	m->itemSel = tp->item;
	tp->panel = AG_MenuExpand(m, tp->item, x+4, y+4);
}

AG_WidgetClass agTlistClass = {
	{
		"AG_Widget:AG_Tlist",
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
