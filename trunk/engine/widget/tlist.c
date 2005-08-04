/*	$Csoft: tlist.c,v 1.126 2005/08/04 06:07:09 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/input.h>

#include "tlist.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <string.h>
#include <stdarg.h>

static struct widget_ops tlist_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tlist_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	tlist_draw,
	tlist_scale
};

enum {
	PAGE_INCREMENT = 4
};

static void tlist_mousebuttondown(int, union evarg *);
static void tlist_keydown(int, union evarg *);
static void tlist_keyup(int, union evarg *);
static void tlist_scrolled(int, union evarg *);
static void free_item(struct tlist *, struct tlist_item *);
static void select_item(struct tlist *, struct tlist_item *);
static void unselect_item(struct tlist *, struct tlist_item *);
static void show_popup(struct tlist *, struct tlist_popup *);

struct tlist *
tlist_new(void *parent, int flags)
{
	struct tlist *tl;

	tl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl, flags);
	object_attach(parent, tl);
	return (tl);
}

/* Displace the selection or scroll in response to keyboard events. */
static void
key_tick(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	struct tlist_item *it, *pit;
	struct widget_binding *offsetb;
	int *offset;

	pthread_mutex_lock(&tl->lock);
	offsetb = widget_get_binding(tl->sbar, "value", &offset);
	switch (keysym) {
	case SDLK_UP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected &&
			    (pit = TAILQ_PREV(it, tlist_itemq, items))
			     != NULL) {
				unselect_item(tl, it);
				select_item(tl, pit);
#if 1
				if (--(*offset) < 0) {
					*offset = 0;
				}
				widget_binding_modified(offsetb);
#endif
				break;
			}
		}
		break;
	case SDLK_DOWN:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected &&
			    (pit = TAILQ_NEXT(it, items)) != NULL) {
				unselect_item(tl, it);
				select_item(tl, pit);
#if 1
				if (++(*offset) >
				    tl->nitems - tl->nvisitems) {
					*offset = tl->nitems -
					    tl->nvisitems;
				}
				widget_binding_modified(offsetb);
#endif
				break;
			}
		}
		break;
	case SDLK_PAGEUP:
		if ((*offset -= PAGE_INCREMENT) < 0) {
			*offset = 0;
		}
		widget_binding_modified(offsetb);
		break;
	case SDLK_PAGEDOWN:
		if ((*offset += PAGE_INCREMENT) > tl->nitems - tl->nvisitems) {
			*offset = tl->nitems - tl->nvisitems;
		}
		widget_binding_modified(offsetb);
		break;
	default:
		break;
	}
	widget_binding_unlock(offsetb);
	pthread_mutex_unlock(&tl->lock);

	tl->keymoved++;
	event_resched(tl, "key-tick", kbd_repeat);
}

static void
dblclick_expire(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;

	pthread_mutex_lock(&tl->lock);
	tl->dblclicked = NULL;
	pthread_mutex_unlock(&tl->lock);
}

static void
lost_focus(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;

	event_cancel(tl, "key-tick");
	event_cancel(tl, "dblclick-expire");
	tl->keymoved = 0;
}

void
tlist_init(struct tlist *tl, int flags)
{
	widget_init(tl, "tlist", &tlist_ops,
	    WIDGET_FOCUSABLE|WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL);
	widget_bind(tl, "selected", WIDGET_POINTER, &tl->selected);

	tl->flags = flags;
	pthread_mutex_init(&tl->lock, &recursive_mutexattr);
	tl->selected = NULL;
	tl->keymoved = 0;
	tl->item_h = text_font_height+2;
	tl->icon_w = tl->item_h - 4;
	tl->dblclicked = NULL;
	tl->nitems = 0;
	tl->nvisitems = 0;
	tl->sbar = scrollbar_new(tl, SCROLLBAR_VERT);
	tl->compare_fn = tlist_compare_ptrs;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);
	TAILQ_INIT(&tl->popups);
	
	tlist_prescale(tl, "XXXXXXXXXXXXXXXXXXXXXXX", 4);

	event_new(tl->sbar, "scrollbar-changed", tlist_scrolled, "%p", tl);
	event_new(tl, "window-mousebuttondown", tlist_mousebuttondown, NULL);
	event_new(tl, "window-keydown", tlist_keydown, NULL);
	event_new(tl, "window-keyup", tlist_keyup, NULL);
	event_new(tl, "key-tick", key_tick, NULL);
	event_new(tl, "dblclick-expire", dblclick_expire, NULL);
	event_new(tl, "widget-lostfocus", lost_focus, NULL);
	event_new(tl, "widget-hidden", lost_focus, NULL);
}

void
tlist_prescale(struct tlist *tl, const char *text, int nitems)
{
	text_prescale(text, &tl->prew, NULL);
	tl->prew += tl->item_h + 5;
	tl->preh = (tl->item_h+2)*nitems;
}

void
tlist_destroy(void *p)
{
	struct tlist *tl = p;
	struct tlist_item *it, *nit;
	struct tlist_popup *tp, *ntp;

	for (it = TAILQ_FIRST(&tl->selitems);
	     it != TAILQ_END(&tl->selitems);
	     it = nit) {
		nit = TAILQ_NEXT(it, selitems);
		free_item(tl, it);
	}
	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		free_item(tl, it);
	}
	for (tp = TAILQ_FIRST(&tl->popups);
	     tp != TAILQ_END(&tl->popups);
	     tp = ntp) {
		ntp = TAILQ_NEXT(tp, popups);
		object_destroy(tp->menu);
		Free(tp->menu, M_OBJECT);
		Free(tp, M_WIDGET);
	}

	pthread_mutex_destroy(&tl->lock);
	widget_destroy(tl);
}

void
tlist_scale(void *p, int w, int h)
{
	struct tlist *tl = p;

	if (w == -1 && h == -1) {
		WIDGET(tl)->w = tl->prew;
		WIDGET(tl)->h = tl->preh;
	}

	WIDGET(tl->sbar)->x = WIDGET(tl)->w - tl->sbar->button_size;
	WIDGET(tl->sbar)->y = 0;

	widget_scale(tl->sbar,
	    tl->sbar->button_size,
	    WIDGET(tl)->h);
}

void
tlist_draw(void *p)
{
	struct tlist *tl = p;
	struct tlist_item *it;
	int y = 0, i = 0;
	int offset;

	if (WIDGET(tl)->w < tl->item_h ||
	    WIDGET(tl)->h < tl->item_h)
		return;

	primitives.box(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h, -1,
	    COLOR(TLIST_BG_COLOR));

	pthread_mutex_lock(&tl->lock);
	if (tl->flags & TLIST_POLL) {
		event_post(NULL, tl, "tlist-poll", NULL);
	}
	offset = widget_get_int(tl->sbar, "value");

	TAILQ_FOREACH(it, &tl->items, items) {
		int x = 2 + it->depth*tl->icon_w;

		if (i++ < offset)
			continue;
		if (y > WIDGET(tl)->h - tl->item_h)
			break;

		if (it->selected) {
			int x1 = x + tl->item_h + 2;

			primitives.rect_filled(tl,
			    x1, y+1,
			    WIDGET(tl)->w-x1-1,
			    tl->item_h-1,
			    COLOR(TLIST_SEL_COLOR));
		}
		
		if (it->iconsrc != NULL) {
			if (it->icon == -1) {
				SDL_Surface *scaled = NULL;

				view_scale_surface(it->iconsrc,
				    tl->item_h, tl->item_h, &scaled);
				it->icon = widget_map_surface(tl, scaled);
			}
			widget_blit_surface(tl, it->icon, x, y);
		}

		if (it->flags & TLIST_HAS_CHILDREN) {
			Uint8 cBg[4] = { 0, 0, 0, 64 };
			Uint8 cFg[4] = { 255, 255, 255, 100 };

			primitives.rect_blended(tl,
			    x-1, y,
			    tl->item_h + 2, tl->item_h,
			    cBg, ALPHA_SRC);

			if (it->flags & TLIST_VISIBLE_CHILDREN) {
				primitives.minus(tl,
				    x+2, y+2,
				    tl->item_h-4, tl->item_h-4,
				    cFg, ALPHA_SRC);
			} else {
				primitives.plus(tl,
				    x+2, y+2,
				    tl->item_h-4, tl->item_h-4,
				    cFg, ALPHA_SRC);
			}
		}
		
		if (it->label == -1) {
			it->label = widget_map_surface(tl,
			    text_render(NULL, -1, COLOR(TLIST_TXT_COLOR),
			        it->text));
		}
		widget_blit_surface(tl, it->label,
		    x + tl->item_h + 5,
		    y + tl->item_h/2 - WIDGET_SURFACE(tl,it->label)->h/2 + 1);

		y += tl->item_h;
		primitives.hline(tl, 0, WIDGET(tl)->w, y,
		    COLOR(TLIST_LINE_COLOR));
	}
	pthread_mutex_unlock(&tl->lock);
}

/* Adjust the scrollbar offset according to the number of visible items. */
static __inline__ void
tlist_adjust_scrollbar(struct tlist *tl)
{
	struct widget_binding *maxb, *offsetb;
	int *max, *offset;
	int noffset;
	
	tl->nvisitems = WIDGET(tl)->h / tl->item_h;

	maxb = widget_get_binding(tl->sbar, "max", &max);
	offsetb = widget_get_binding(tl->sbar, "value", &offset);
	noffset = *offset;

	*max = tl->nitems - tl->nvisitems;

	if (noffset > *max)
		noffset = *max;
	if (noffset < 0)
		noffset = 0;

	if (*offset != noffset) {
		*offset = noffset;
		widget_binding_modified(offsetb);
	}

	if (tl->nitems > 0 && tl->nvisitems > 0 &&
	    tl->nvisitems < tl->nitems) {
		scrollbar_set_bar_size(tl->sbar,
		    tl->nvisitems *
		    (WIDGET(tl->sbar)->h - tl->sbar->button_size*2) /
		    tl->nitems);
	} else {
		scrollbar_set_bar_size(tl->sbar, -1);		/* Full range */
	}

	widget_binding_unlock(offsetb);
	widget_binding_unlock(maxb);
}

static void
free_item(struct tlist *tl, struct tlist_item *it)
{
	if (it->label != -1)
		widget_unmap_surface(tl, it->label);
	if (it->icon != -1)
		widget_unmap_surface(tl, it->icon);

	Free(it, M_WIDGET);
}

void
tlist_remove_item(struct tlist *tl, struct tlist_item *it)
{
	struct widget_binding *offsetb;
	int *offset;
	int nitems;

	pthread_mutex_lock(&tl->lock);
	TAILQ_REMOVE(&tl->items, it, items);
	nitems = --tl->nitems;
	pthread_mutex_unlock(&tl->lock);

	free_item(tl, it);

	/* Update the scrollbar range and offset accordingly. */
	widget_set_int(tl->sbar, "max", nitems);
	offsetb = widget_get_binding(tl->sbar, "value", &offset);
	if (*offset > nitems) {
		*offset = nitems;
	} else if (nitems > 0 && *offset < nitems) {		/* XXX ugly */
		*offset = 0;
	}
	widget_binding_unlock(offsetb);
}

/* Clear the items on the list, save the selections if polling. */
void
tlist_clear_items(struct tlist *tl)
{
	struct tlist_item *it, *nit;
	
	pthread_mutex_lock(&tl->lock);

	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if (it->selected || (it->flags & TLIST_HAS_CHILDREN)) {
			TAILQ_INSERT_HEAD(&tl->selitems, it, selitems);
		} else {
			free_item(tl, it);
		}
	}
	TAILQ_INIT(&tl->items);
	tl->nitems = 0;
	
	/* Preserve the offset value, for polling. */
	widget_set_int(tl->sbar, "max", 0);

	pthread_mutex_unlock(&tl->lock);
}

int
tlist_compare_strings(const struct tlist_item *it1,
    const struct tlist_item *it2)
{
	return (it1->text != NULL && it2->text != NULL &&
	        strcmp(it1->text, it2->text) == 0);
}

int
tlist_compare_ptrs(const struct tlist_item *it1, const struct tlist_item *it2)
{
	return (it1->p1 == it2->p1);
}

int
tlist_compare_ptrs_classes(const struct tlist_item *it1,
    const struct tlist_item *it2)
{
	return ((it1->p1 == it2->p1) &&
	        (it1->class != NULL && it2->class != NULL &&
		 (strcmp(it1->class, it2->class) == 0)));
}

void
tlist_set_compare_fn(struct tlist *tl,
    int (*fn)(const struct tlist_item *, const struct tlist_item *))
{
	tl->compare_fn = fn;
}

/* Restore previous item selection state. */
void
tlist_restore_selections(struct tlist *tl)
{
	struct tlist_item *sit, *cit, *nsit;

	for (sit = TAILQ_FIRST(&tl->selitems);
	     sit != TAILQ_END(&tl->selitems);
	     sit = nsit) {
		nsit = TAILQ_NEXT(sit, selitems);
		TAILQ_FOREACH(cit, &tl->items, items) {
			if (!tl->compare_fn(sit, cit)) {
				continue;
			}
			cit->selected = sit->selected;
			if (sit->flags & TLIST_VISIBLE_CHILDREN) {
				cit->flags |= TLIST_VISIBLE_CHILDREN;
			} else {
				cit->flags &= ~(TLIST_VISIBLE_CHILDREN);
			}
		}
		free_item(tl, sit);
	}
	TAILQ_INIT(&tl->selitems);

	tlist_adjust_scrollbar(tl);
}

/*
 * See if the child items of an item are supposed to be visible; called from
 * polling routines when displaying trees.
 */
int
tlist_visible_children(struct tlist *tl, struct tlist_item *cit)
{
	struct tlist_item *sit;

	TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tl->compare_fn(sit, cit))
			break;
	}
	if (sit == NULL) 
		return (0);				/* Default */

	return (sit->flags & TLIST_VISIBLE_CHILDREN);
}

/*
 * Allocate a new tlist item.
 * XXX allocate from a pool, especially for polled items.
 */
static __inline__ struct tlist_item *
allocate_item(struct tlist *tl, SDL_Surface *iconsrc)
{
	struct tlist_item *it;

	it = Malloc(sizeof(struct tlist_item), M_WIDGET);
	it->selected = 0;
	it->class = "";
	it->depth = 0;
	it->flags = 0;
	it->icon = -1;
	it->label = -1;
	tlist_set_icon(tl, it, iconsrc);
	return (it);
}

static __inline__ void
insert_item(struct tlist *tl, struct tlist_item *it, int ins_head)
{
	pthread_mutex_lock(&tl->lock);
	if (ins_head) {
		TAILQ_INSERT_HEAD(&tl->items, it, items);
	} else {
		TAILQ_INSERT_TAIL(&tl->items, it, items);
	}
	widget_set_int(tl->sbar, "max", ++tl->nitems);
	pthread_mutex_unlock(&tl->lock);
}

/* Add an item to the list. */
struct tlist_item *
tlist_insert_item(struct tlist *tl, SDL_Surface *iconsrc, const char *text,
    void *p1)
{
	struct tlist_item *it;

	it = allocate_item(tl, iconsrc);
	it->p1 = p1;
	strlcpy(it->text, text, sizeof(it->text));

	insert_item(tl, it, 0);
	return (it);
}

struct tlist_item *
tlist_insert(struct tlist *tl, SDL_Surface *iconsrc, const char *fmt, ...)
{
	struct tlist_item *it;
	va_list args;
	
	it = allocate_item(tl, iconsrc);
	it->p1 = NULL;

	va_start(args, fmt);
	vsnprintf(it->text, sizeof(it->text), fmt, args);
	va_end(args);

	insert_item(tl, it, 0);
	return (it);
}

struct tlist_item *
tlist_insert_item_head(struct tlist *tl, SDL_Surface *icon, const char *text,
    void *p1)
{
	struct tlist_item *it;

	it = allocate_item(tl, icon);
	it->p1 = p1;
	strlcpy(it->text, text, sizeof(it->text));

	insert_item(tl, it, 1);
	return (it);
}

/* Select an item based on its pointer value. */
struct tlist_item *
tlist_select_pointer(struct tlist *tl, void *p)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	if (tl->flags & TLIST_POLL) {
		event_post(NULL, tl, "tlist-poll", NULL);
	}
	if ((tl->flags & TLIST_MULTI) == 0) {
		tlist_unselect_all(tl);
	}
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->p1 == p) {
			it->selected++;
			break;
		}
	}
	pthread_mutex_unlock(&tl->lock);
	return (it);
}

/* Select an item based on text. */
struct tlist_item *
tlist_select_text(struct tlist *tl, const char *text)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	if (tl->flags & TLIST_POLL) {
		event_post(NULL, tl, "tlist-poll", NULL);
	}
	if ((tl->flags & TLIST_MULTI) == 0) {
		tlist_unselect_all(tl);
	}
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->text[0] == text[0] &&
		    strcmp(it->text, text) == 0) {
			it->selected++;
			break;
		}
	}
	pthread_mutex_unlock(&tl->lock);
	return (it);
}

/* Set the selection flag on an item. */
void
tlist_select(struct tlist *tl, struct tlist_item *it)
{
	pthread_mutex_lock(&tl->lock);
	if ((tl->flags & TLIST_MULTI) == 0) {
		tlist_unselect_all(tl);
	}
	it->selected = 1;
	pthread_mutex_unlock(&tl->lock);
}

/* Clear the selection flag on an item. */
void
tlist_unselect(struct tlist *tl, struct tlist_item *it)
{
	pthread_mutex_lock(&tl->lock);
	it->selected = 0;
	pthread_mutex_unlock(&tl->lock);
}

/* Set the selection flag on all items. */
void
tlist_select_all(struct tlist *tl)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		it->selected = 1;
	}
	pthread_mutex_unlock(&tl->lock);
}

/* Unset the selection flag on all items. */
void
tlist_unselect_all(struct tlist *tl)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		it->selected = 0;
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
select_item(struct tlist *tl, struct tlist_item *it)
{
	struct widget_binding *selectedb;
	void **selected;

	selectedb = widget_get_binding(tl, "selected", &selected);
	*selected = it->p1;
	it->selected++;
	event_post(NULL, tl, "tlist-changed", "%p, %i", it, 1);
	event_post(NULL, tl, "tlist-selected", "%p", it);
	widget_binding_modified(selectedb);
	widget_binding_unlock(selectedb);
}

static void
unselect_item(struct tlist *tl, struct tlist_item *it)
{
	struct widget_binding *selectedb;
	void **selected;

	selectedb = widget_get_binding(tl, "selected", &selected);
	*selected = NULL;
	it->selected = 0;
	event_post(NULL, tl, "tlist-changed", "%p, %i", it, 0);
	widget_binding_modified(selectedb);
	widget_binding_unlock(selectedb);
}

static void
tlist_mousebuttondown(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct tlist_item *ti;
	int tind;

	pthread_mutex_lock(&tl->lock);

	tind = (widget_get_int(tl->sbar, "value") + y/tl->item_h) + 1;

	/* XXX use array */
	if ((ti = tlist_find_index(tl, tind)) == NULL)
		goto out;

	/* Expand the children if the user clicked on the [+] sign. */
	if (ti->flags & TLIST_HAS_CHILDREN) {
		int th = tl->icon_w;

		x -= 7;
		if (x >= ti->depth*th &&
		    x <= (ti->depth+1)*th) {
			if (ti->flags & TLIST_VISIBLE_CHILDREN) {
				ti->flags &= ~(TLIST_VISIBLE_CHILDREN);
			} else {
				ti->flags |= TLIST_VISIBLE_CHILDREN;
			}
			goto out;
		}
	}

	switch (button) {
	case SDL_BUTTON_LEFT:
		/* Handle range selections. */
		if ((tl->flags & TLIST_MULTI) &&
		    (SDL_GetModState() & KMOD_SHIFT)) {
			struct tlist_item *oitem;
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
					if (i > oind)
						oitem->selected = 1;
					i++;
				}
			} else if (oind >= tind) {		  /* Backward */
				i = nitems;
				TAILQ_FOREACH_REVERSE(oitem, &tl->items,
				    items, tlist_itemq) {
					if (i <= oind)
						oitem->selected = 1;
					if (i == tind)
						break;
					i--;
				}
			}
			break;
		}

		/* Handle single selections. */
		if ((tl->flags & TLIST_MULTITOGGLE) ||
		    ((tl->flags & TLIST_MULTI) &&
		     (SDL_GetModState() & KMOD_CTRL))) {
		    	ti->selected = !ti->selected;
			break;
		}

		widget_focus(tl);
		tlist_unselect_all(tl);
		select_item(tl, ti);

		/* Handle double clicks. */
		if (tl->dblclicked != NULL && tl->dblclicked == ti->p1) {
			event_cancel(tl, "dblclick-expire");
			event_post(NULL, tl, "tlist-dblclick", "%p", ti->p1);
			tl->dblclicked = NULL;
		} else {
			tl->dblclicked = ti->p1;
			event_schedule(NULL, tl, mouse_dblclick_delay,
			    "dblclick-expire", "%p", ti);
		}
		break;
	case SDL_BUTTON_RIGHT:
		if (ti->class != NULL) {
			struct tlist_popup *tp;
	
			widget_focus(tl);
		
			if (!(tl->flags & (TLIST_MULTITOGGLE|TLIST_MULTI)) ||
			    !(SDL_GetModState() & (KMOD_CTRL|KMOD_SHIFT))) {
				tlist_unselect_all(tl);
				select_item(tl, ti);
			}
	
			TAILQ_FOREACH(tp, &tl->popups, popups) {
				if (strcmp(tp->iclass, ti->class) == 0)
					break;
			}
			if (tp != NULL) {
				show_popup(tl, tp);
				goto out;
			}
		}
		break;
	}
out:
	pthread_mutex_unlock(&tl->lock);
}

static void
tlist_keydown(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&tl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		event_schedule(NULL, tl, kbd_delay, "key-tick", "%i", keysym);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
tlist_keyup(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&tl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		if (tl->keymoved == 0) {
			event_post(NULL, tl, "key-tick", "%i", keysym);
		}
		event_cancel(tl, "key-tick");
		tl->keymoved = 0;
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
tlist_scrolled(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;

	tlist_adjust_scrollbar(tl);
}

/*
 * Return the item at the given index.
 * The tlist must be locked.
 */
struct tlist_item *
tlist_find_index(struct tlist *tl, int index)
{
	struct tlist_item *it;
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
struct tlist_item *
tlist_selected_item(struct tlist *tl)
{
	struct tlist_item *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected)
			return (it);
	}
	return (NULL);
}

/*
 * Return the pointer associated with the first selected item.
 * The tlist must be locked.
 */
void *
tlist_find_pointer(struct tlist *tl)
{
	struct tlist_item *it;

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
struct tlist_item *
tlist_find_text(struct tlist *tl, const char *text)
{
	struct tlist_item *it;

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
struct tlist_item *
tlist_first_item(struct tlist *tl)
{
	return (TAILQ_FIRST(&tl->items));
}

/*
 * Return the last item on the list.
 * The tlist must be locked.
 */
struct tlist_item *
tlist_last_item(struct tlist *tl)
{
	return (TAILQ_LAST(&tl->items, tlist_itemq));
}

/* Set the height to use for item display. */
void
tlist_set_item_height(struct tlist *tl, int ih)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	tl->item_h = ih;
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != -1) {
			SDL_Surface *scaled = NULL;

			view_scale_surface(it->iconsrc,
			    tl->item_h-1, tl->item_h-1, &scaled);
			widget_replace_surface(tl, it->icon, scaled);
		}
	}
	pthread_mutex_unlock(&tl->lock);
}

/* Update the icon associated with an item. */
void
tlist_set_icon(struct tlist *tl, struct tlist_item *it, SDL_Surface *iconsrc)
{
	it->iconsrc = iconsrc;

	if (it->icon != -1) {
		widget_unmap_surface(tl, it->icon);
		it->icon = -1;
	}
}

/* Create a new popup menu for items of the given class. */
struct AGMenuItem *
tlist_set_popup(struct tlist *tl, const char *iclass)
{
	struct tlist_popup *tp;

	tp = Malloc(sizeof(struct tlist_popup), M_WIDGET);
	tp->iclass = iclass;
	tp->panel = NULL;

	tp->menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(tp->menu);

	tp->item = menu_add_item(tp->menu, iclass);

	TAILQ_INSERT_TAIL(&tl->popups, tp, popups);
	return (tp->item);
}

static void
show_popup(struct tlist *tl, struct tlist_popup *tp)
{
	struct AGMenu *m = tp->menu;
	int x, y;

#if 0
	if (widget_parent_window(tl) == NULL)
		fatal("%s is unattached", OBJECT(tl)->name);
#endif
	mouse_get_state(&x, &y);

	if (tp->panel != NULL) {
		menu_collapse(m, tp->item);
		tp->panel = NULL;
	}
#if 0
	if (m->sel_item != NULL) {
		menu_collapse(m, m->sel_item);
	}
#endif
	m->sel_item = tp->item;
	tp->panel = menu_expand(m, tp->item, x+4, y+4);
}

