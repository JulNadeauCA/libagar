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
#include <core/view.h>

#include "tlist.h"

#include "primitive.h"

#include <string.h>
#include <stdarg.h>

static void mousebuttondown(AG_Event *);
static void keydown(AG_Event *);
static void keyup(AG_Event *);

static void FreeItem(AG_Tlist *, AG_TlistItem *);
static void SelectItem(AG_Tlist *, AG_TlistItem *);
static void DeselectItem(AG_Tlist *, AG_TlistItem *);
static void PopupMenu(AG_Tlist *, AG_TlistPopup *);
static void UpdateItemIcon(AG_Tlist *, AG_TlistItem *, SDL_Surface *);
static void UpdateListScrollbar(AG_Tlist *);
static void ScrollbarChanged(AG_Event *);

AG_Tlist *
AG_TlistNew(void *parent, Uint flags)
{
	AG_Tlist *tl;

	tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl, flags);
	AG_ObjectAttach(parent, tl);
	if (flags & AG_TLIST_FOCUS) {
		AG_WidgetFocus(tl);
	}
	return (tl);
}

AG_Tlist *
AG_TlistNewPolled(void *parent, Uint flags, AG_EventFn fn, const char *fmt, ...)
{
	AG_Tlist *tl;
	AG_Event *ev;

	tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl, flags|AG_TLIST_POLL);
	ev = AG_SetEvent(tl, "tlist-poll", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	AG_ObjectAttach(parent, tl);
	return (tl);
}

/* Displace the selection or scroll in response to keyboard events. */
static void
KeyTimeout(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	AG_TlistItem *it, *pit;
	AG_WidgetBinding *offsetb;
	int *offset;

	AG_MutexLock(&tl->lock);
	offsetb = AG_WidgetGetBinding(tl->sbar, "value", &offset);
	switch (keysym) {
	case SDLK_UP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected &&
			    (pit = TAILQ_PREV(it, ag_tlist_itemq, items))
			     != NULL) {
				DeselectItem(tl, it);
				SelectItem(tl, pit);
#if 1
				if (--(*offset) < 0) {
					*offset = 0;
				}
				AG_WidgetBindingChanged(offsetb);
#endif
				break;
			}
		}
		break;
	case SDLK_DOWN:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected &&
			    (pit = TAILQ_NEXT(it, items)) != NULL) {
				DeselectItem(tl, it);
				SelectItem(tl, pit);
#if 1
				if (++(*offset) >
				    tl->nitems - tl->nvisitems) {
					*offset = tl->nitems -
					    tl->nvisitems;
				}
				AG_WidgetBindingChanged(offsetb);
#endif
				break;
			}
		}
		break;
	case SDLK_PAGEUP:
		if ((*offset -= agPageIncrement) < 0) {
			*offset = 0;
		}
		AG_WidgetBindingChanged(offsetb);
		break;
	case SDLK_PAGEDOWN:
		if ((*offset += agPageIncrement) > tl->nitems - tl->nvisitems) {
			*offset = tl->nitems - tl->nvisitems;
		}
		AG_WidgetBindingChanged(offsetb);
		break;
	default:
		break;
	}
	AG_WidgetUnlockBinding(offsetb);
	AG_MutexUnlock(&tl->lock);

	tl->keymoved++;
	AG_ReschedEvent(tl, "key-tick", agKbdRepeat);
}

static void
DoubleClickTimeout(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	AG_MutexLock(&tl->lock);
	tl->dblclicked = NULL;
	AG_MutexUnlock(&tl->lock);
}

static void
LostFocus(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();

	AG_CancelEvent(tl, "key-tick");
	AG_CancelEvent(tl, "dblclick-expire");
	tl->keymoved = 0;
}

void
AG_TlistInit(AG_Tlist *tl, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	if (flags & AG_TLIST_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_TLIST_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(tl, &agTlistOps, wflags);
	AG_WidgetBind(tl, "selected", AG_WIDGET_POINTER, &tl->selected);

	tl->flags = flags;
	AG_MutexInitRecursive(&tl->lock);
	tl->selected = NULL;
	tl->keymoved = 0;
	tl->item_h = agTextFontHeight+2;
	tl->icon_w = tl->item_h - 4;
	tl->dblclicked = NULL;
	tl->nitems = 0;
	tl->nvisitems = 0;
	tl->sbar = AG_ScrollbarNew(tl, AG_SCROLLBAR_VERT, 0);
	tl->compare_fn = AG_TlistComparePtrs;
	tl->prew = tl->item_h + 5;
	tl->preh = tl->item_h + 2;
	tl->popupEv = NULL;
	tl->dblClickEv = NULL;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	TAILQ_INIT(&tl->popups);

	AG_SetEvent(tl->sbar, "scrollbar-changed", ScrollbarChanged, "%p", tl);
	AG_SetEvent(tl, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(tl, "window-keydown", keydown, NULL);
	AG_SetEvent(tl, "window-keyup", keyup, NULL);
	AG_SetEvent(tl, "key-tick", KeyTimeout, NULL);
	AG_SetEvent(tl, "dblclick-expire", DoubleClickTimeout, NULL);
	AG_SetEvent(tl, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(tl, "widget-hidden", LostFocus, NULL);
}

void
AG_TlistPrescale(AG_Tlist *tl, const char *text, int nitems)
{
	AG_TextSize(text, &tl->prew, NULL);
	tl->prew += tl->item_h + 5;
	tl->preh = (tl->item_h+2)*nitems;
}

void
AG_TlistDestroy(void *p)
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
		Free(tp->menu, M_OBJECT);
		Free(tp, M_WIDGET);
	}

	AG_MutexDestroy(&tl->lock);
	AG_WidgetDestroy(tl);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Tlist *tl = p;

	r->w = tl->prew;
	r->h = tl->preh;
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

	if (a->w < rBar.w || a->h < tl->sbar->bw*2) {
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

	agPrim.box(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h, -1,
	    AG_COLOR(TLIST_BG_COLOR));
	
	AG_MutexLock(&tl->lock);
	if (tl->flags & AG_TLIST_POLL) {
		AG_PostEvent(NULL, tl, "tlist-poll", NULL);
	}
	offset = AG_WidgetInt(tl->sbar, "value");

	TAILQ_FOREACH(it, &tl->items, items) {
		int x = 2 + it->depth*tl->icon_w;

		if (i++ < offset)
			continue;
		if (y > WIDGET(tl)->h - tl->item_h)
			break;

		if (it->selected) {
			int x1 = x + tl->item_h + 2;

			agPrim.rect_filled(tl,
			    x1, y,
			    WIDGET(tl)->w-x1,
			    tl->item_h,
			    AG_COLOR(TLIST_SEL_COLOR));
		}
		
		if (it->iconsrc != NULL) {
			if (it->icon == -1) {
				SDL_Surface *scaled = NULL;

				AG_ScaleSurface(it->iconsrc,
				    tl->item_h, tl->item_h, &scaled);
				it->icon = AG_WidgetMapSurface(tl, scaled);
			}
			AG_WidgetBlitSurface(tl, it->icon, x, y);
		}

		if (it->flags & AG_TLIST_HAS_CHILDREN) {
			Uint8 cBg[4] = { 0, 0, 0, 64 };
			Uint8 cFg[4] = { 255, 255, 255, 100 };

			agPrim.rect_blended(tl,
			    x-1, y,
			    tl->item_h + 2, tl->item_h,
			    cBg, AG_ALPHA_SRC);

			if (it->flags & AG_TLIST_VISIBLE_CHILDREN) {
				agPrim.minus(tl,
				    x+2, y+2,
				    tl->item_h-4, tl->item_h-4,
				    cFg, AG_ALPHA_SRC);
			} else {
				agPrim.plus(tl,
				    x+2, y+2,
				    tl->item_h-4, tl->item_h-4,
				    cFg, AG_ALPHA_SRC);
			}
		}
		
		if (it->label == -1) {
			AG_TextColor(TLIST_TXT_COLOR);
			it->label = AG_WidgetMapSurface(tl,
			    AG_TextRender(it->text));
		}
		AG_WidgetBlitSurface(tl, it->label,
		    x + tl->item_h + 5,
		    y + tl->item_h/2 - WSURFACE(tl,it->label)->h/2 + 2);

		y += tl->item_h;
		if (y < WIDGET(tl)->h - 1) {
			agPrim.hline(tl, 0, WIDGET(tl)->w, y,
			    AG_COLOR(TLIST_LINE_COLOR));
		}
	}
	AG_MutexUnlock(&tl->lock);
}

/* Adjust the scrollbar offset according to the number of visible items. */
static void
UpdateListScrollbar(AG_Tlist *tl)
{
	AG_WidgetBinding *maxb, *offsetb;
	int *max, *offset;
	int noffset;
	
	tl->nvisitems = WIDGET(tl)->h / tl->item_h;

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
		    tl->nvisitems *
		    (WIDGET(tl->sbar)->h - tl->sbar->bw*2)/tl->nitems);
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
		SDL_FreeSurface(it->iconsrc);
	}
	if (it->icon != -1)
		AG_WidgetUnmapSurface(tl, it->icon);

	Free(it, M_WIDGET);
}

void
AG_TlistDel(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_WidgetBinding *offsetb;
	int *offset;
	int nitems;

	AG_MutexLock(&tl->lock);
	TAILQ_REMOVE(&tl->items, it, items);
	nitems = --tl->nitems;
	AG_MutexUnlock(&tl->lock);

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
}

/* Clear the items on the list, save the selections if polling. */
void
AG_TlistClear(AG_Tlist *tl)
{
	AG_TlistItem *it, *nit;
	
	AG_MutexLock(&tl->lock);

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

	AG_MutexUnlock(&tl->lock);
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
	tl->compare_fn = fn;
}

/* Restore previous item selection state. */
void
AG_TlistRestore(AG_Tlist *tl)
{
	AG_TlistItem *sit, *cit, *nsit;

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
}

/*
 * See if the child items of an item are supposed to be visible; called from
 * polling routines when displaying trees.
 */
int
AG_TlistVisibleChildren(AG_Tlist *tl, AG_TlistItem *cit)
{
	AG_TlistItem *sit;

	TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tl->compare_fn(sit, cit))
			break;
	}
	if (sit == NULL) 
		return (0);				/* Default */

	return (sit->flags & AG_TLIST_VISIBLE_CHILDREN);
}

/*
 * Allocate a new tlist item.
 * XXX allocate from a pool, especially for polled items.
 */
static __inline__ AG_TlistItem *
AllocItem(AG_Tlist *tl, SDL_Surface *iconsrc)
{
	AG_TlistItem *it;

	it = Malloc(sizeof(AG_TlistItem), M_WIDGET);
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

static __inline__ void
insert_item(AG_Tlist *tl, AG_TlistItem *it, int ins_head)
{
	AG_MutexLock(&tl->lock);
	if (ins_head) {
		TAILQ_INSERT_HEAD(&tl->items, it, items);
	} else {
		TAILQ_INSERT_TAIL(&tl->items, it, items);
	}
	AG_WidgetSetInt(tl->sbar, "max", ++tl->nitems);
	AG_MutexUnlock(&tl->lock);
}

/* Add an item to the list. */
AG_TlistItem *
AG_TlistAddPtr(AG_Tlist *tl, SDL_Surface *iconsrc, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	it = AllocItem(tl, iconsrc);
	it->p1 = p1;
	strlcpy(it->text, text, sizeof(it->text));

	insert_item(tl, it, 0);
	return (it);
}

AG_TlistItem *
AG_TlistAdd(AG_Tlist *tl, SDL_Surface *iconsrc, const char *fmt, ...)
{
	AG_TlistItem *it;
	va_list args;
	
	it = AllocItem(tl, iconsrc);
	it->p1 = NULL;

	va_start(args, fmt);
	vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);

	insert_item(tl, it, 0);
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
AG_TlistAddPtrHead(AG_Tlist *tl, SDL_Surface *icon, const char *text,
    void *p1)
{
	AG_TlistItem *it;

	it = AllocItem(tl, icon);
	it->p1 = p1;
	strlcpy(it->text, text, sizeof(it->text));

	insert_item(tl, it, 1);
	return (it);
}

/* Select an item based on its pointer value. */
AG_TlistItem *
AG_TlistSelectPtr(AG_Tlist *tl, void *p)
{
	AG_TlistItem *it;

	AG_MutexLock(&tl->lock);
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
	AG_MutexUnlock(&tl->lock);
	return (it);
}

/* Select an item based on text. */
AG_TlistItem *
AG_TlistSelectText(AG_Tlist *tl, const char *text)
{
	AG_TlistItem *it;

	AG_MutexLock(&tl->lock);
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
	AG_MutexUnlock(&tl->lock);
	return (it);
}

/* Set the selection flag on an item. */
void
AG_TlistSelect(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_MutexLock(&tl->lock);
	if ((tl->flags & AG_TLIST_MULTI) == 0) {
		AG_TlistDeselectAll(tl);
	}
	SelectItem(tl, it);
	AG_MutexUnlock(&tl->lock);
}

/* Clear the selection flag on an item. */
void
AG_TlistDeselect(AG_Tlist *tl, AG_TlistItem *it)
{
	DeselectItem(tl, it);
}

/* Set the selection flag on all items. */
void
AG_TlistSelectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_MutexLock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		SelectItem(tl, it);
	}
	AG_MutexUnlock(&tl->lock);
}

/* Unset the selection flag on all items. */
void
AG_TlistDeselectAll(AG_Tlist *tl)
{
	AG_TlistItem *it;

	AG_MutexLock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		DeselectItem(tl, it);
	}
	AG_MutexUnlock(&tl->lock);
}

static void
SelectItem(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_WidgetBinding *selectedb;
	void **sel_ptr;

	selectedb = AG_WidgetGetBinding(tl, "selected", &sel_ptr);
	*sel_ptr = it->p1;
	if (!it->selected) {
		it->selected = 1;
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 1);
	}
	AG_PostEvent(NULL, tl, "tlist-selected", "%p", it);
	AG_WidgetBindingChanged(selectedb);
	AG_WidgetUnlockBinding(selectedb);
}

static void
DeselectItem(AG_Tlist *tl, AG_TlistItem *it)
{
	AG_WidgetBinding *selectedb;
	void **sel_ptr;

	selectedb = AG_WidgetGetBinding(tl, "selected", &sel_ptr);
	*sel_ptr = NULL;
	if (it->selected) {
		it->selected = 0;
		AG_PostEvent(NULL, tl, "tlist-changed", "%p, %i", it, 0);
	}
	AG_WidgetBindingChanged(selectedb);
	AG_WidgetUnlockBinding(selectedb);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_TlistItem *ti;
	int tind;

	AG_MutexLock(&tl->lock);

	tind = (AG_WidgetInt(tl->sbar, "value") + y/tl->item_h) + 1;

	/* XXX use array */
	if ((ti = AG_TlistFindByIndex(tl, tind)) == NULL)
		goto out;

	/* Expand the children if the user clicked on the [+] sign. */
	if (ti->flags & AG_TLIST_HAS_CHILDREN) {
		int th = tl->icon_w;

		x -= 7;
		if (x >= ti->depth*th &&
		    x <= (ti->depth+1)*th) {
			if (ti->flags & AG_TLIST_VISIBLE_CHILDREN) {
				ti->flags &= ~(AG_TLIST_VISIBLE_CHILDREN);
			} else {
				ti->flags |= AG_TLIST_VISIBLE_CHILDREN;
			}
			goto out;
		}
	}

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		{
			static Uint32 t = 0;
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(tl->sbar, "value", &offs);
			if (((*offs) -= AG_WidgetScrollDelta(&t)) < 0) {
				*offs = 0;
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		{
			static Uint32 t = 0;
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(tl->sbar, "value", &offs);
			if (((*offs) += AG_WidgetScrollDelta(&t)) >
			    (tl->nitems - tl->nvisitems)) {
				*offs = tl->nitems - tl->nvisitems;
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_LEFT:
		if (ti->flags & AG_TLIST_NO_SELECT) {
			goto out;
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
				goto out;
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
			goto out;
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
				goto out;
			}
		}
		break;
	}
out:
	AG_MutexUnlock(&tl->lock);
}

static void
keydown(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&tl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		AG_SchedEvent(NULL, tl, agKbdDelay, "key-tick", "%i", keysym);
		break;
	default:
		break;
	}
	AG_MutexUnlock(&tl->lock);
}

static void
keyup(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&tl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		if (tl->keymoved == 0) {
			AG_PostEvent(NULL, tl, "key-tick", "%i", keysym);
		}
		AG_CancelEvent(tl, "key-tick");
		tl->keymoved = 0;
		break;
	default:
		break;
	}
	AG_MutexUnlock(&tl->lock);
}

static void
ScrollbarChanged(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);

	UpdateListScrollbar(tl);
}

/*
 * Return the item at the given index.
 * The tlist must be locked.
 */
AG_TlistItem *
AG_TlistFindByIndex(AG_Tlist *tl, int index)
{
	AG_TlistItem *it;
	int i = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (++i == index)
			return (it);
	}
	return (NULL);
}

/*
 * Return the first selected item.
 * The tlist must be locked.
 */
AG_TlistItem *
AG_TlistSelectedItem(AG_Tlist *tl)
{
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected)
			return (it);
	}
	return (NULL);
}

/*
 * Return the user pointer of the first selected item.
 * The tlist must be locked.
 */
void *
AG_TlistSelectedItemPtr(AG_Tlist *tl)
{
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected)
			return (it->p1);
	}
	return (NULL);
}

/*
 * Return the pointer associated with the first selected item.
 * The tlist must be locked.
 */
void *
AG_TlistFindPtr(AG_Tlist *tl)
{
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected)
			return (it->p1);
	}
	return (NULL);
}

/*
 * Return the first item matching a text string.
 * The tlist must be locked.
 */
AG_TlistItem *
AG_TlistFindText(AG_Tlist *tl, const char *text)
{
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (strcmp(it->text, text) == 0)
			return (it);
	}
	return (NULL);
}

/*
 * Return the first item on the list.
 * The tlist must be locked.
 */
AG_TlistItem *
AG_TlistFirstItem(AG_Tlist *tl)
{
	return (TAILQ_FIRST(&tl->items));
}

/*
 * Return the last item on the list.
 * The tlist must be locked.
 */
AG_TlistItem *
AG_TlistLastItem(AG_Tlist *tl)
{
	return (TAILQ_LAST(&tl->items, ag_tlist_itemq));
}

/* Set the height to use for item display. */
void
AG_TlistSetItemHeight(AG_Tlist *tl, int ih)
{
	AG_TlistItem *it;

	AG_MutexLock(&tl->lock);
	tl->item_h = ih;
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != -1) {
			SDL_Surface *scaled = NULL;

			AG_ScaleSurface(it->iconsrc,
			    tl->item_h-1, tl->item_h-1, &scaled);
			AG_WidgetReplaceSurface(tl, it->icon, scaled);
		}
	}
	AG_MutexUnlock(&tl->lock);
}

/* Update the icon associated with an item. */
static void
UpdateItemIcon(AG_Tlist *tl, AG_TlistItem *it, SDL_Surface *iconsrc)
{
	if (it->flags & AG_TLIST_DYNICON) {
		if (it->iconsrc != NULL) {
			SDL_FreeSurface(it->iconsrc);
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
AG_TlistSetIcon(AG_Tlist *tl, AG_TlistItem *it, SDL_Surface *iconsrc)
{
	it->flags |= AG_TLIST_DYNICON;
	UpdateItemIcon(tl, it, iconsrc);
}

void
AG_TlistSetDblClickFn(AG_Tlist *tl, void (*ev)(AG_Event *), const char *fmt,
    ...)
{
	AG_MutexLock(&tl->lock);
	tl->dblClickEv = AG_SetEvent(tl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tl->dblClickEv, fmt);
	AG_MutexUnlock(&tl->lock);
}

void
AG_TlistSetPopupFn(AG_Tlist *tl, void (*ev)(AG_Event *), const char *fmt, ...)
{
	AG_MutexLock(&tl->lock);
	tl->popupEv = AG_SetEvent(tl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tl->popupEv, fmt);
	AG_MutexUnlock(&tl->lock);
}

/* Create a new popup menu for items of the given class. */
AG_MenuItem *
AG_TlistSetPopup(AG_Tlist *tl, const char *iclass)
{
	AG_TlistPopup *tp;

	tp = Malloc(sizeof(AG_TlistPopup), M_WIDGET);
	tp->iclass = iclass;
	tp->panel = NULL;
	tp->menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(tp->menu, 0);
	tp->item = tp->menu->root;		/* XXX redundant */

	TAILQ_INSERT_TAIL(&tl->popups, tp, popups);
	return (tp->item);
}

static void
PopupMenu(AG_Tlist *tl, AG_TlistPopup *tp)
{
	AG_Menu *m = tp->menu;
	int x, y;

#if 0
	if (AG_WidgetParentWindow(tl) == NULL)
		fatal("%s is unattached", OBJECT(tl)->name);
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

const AG_WidgetOps agTlistOps = {
	{
		"AG_Widget:AG_Tlist",
		sizeof(AG_Tlist),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_TlistDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
