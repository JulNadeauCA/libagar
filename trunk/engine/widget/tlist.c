/*	$Csoft: tlist.c,v 1.58 2003/05/22 05:43:12 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/strlcpy.h>

#include <engine/engine.h>
#include <engine/view.h>

#include "tlist.h"

#include <engine/widget/primitive.h>
#include <engine/widget/text.h>
#include <engine/widget/region.h>
#include <engine/widget/window.h>

#include <string.h>

static struct widget_ops tlist_ops = {
	{
		NULL,		/* init */
		tlist_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	tlist_draw,
	NULL		/* update */
};

enum {
	TEXT_COLOR,
	BACKGROUND_COLOR,
	LINE_COLOR,
	SELECTION_COLOR
};

enum {
	SELECTION_MOUSE_BUTTON = 1,
	DEFAULT_BUTTON_SIZE =	 20,
	PAGE_INCREMENT =	 4,
	DBLCLICK_DELAY =	 250,
	MIN_WIDTH =		 26,
	MIN_HEIGHT =		 5
};

static void	tlist_mousebuttondown(int, union evarg *);
static void	tlist_mousebuttonup(int, union evarg *);
static void	tlist_mousemotion(int, union evarg *);
static void	tlist_keydown(int, union evarg *);
static void	tlist_scaled(int, union evarg *);
static void	tlist_attached(int, union evarg *);
static void	tlist_free_item(struct tlist_item *);
static void	tlist_adjust_scrollbar(struct tlist *);

struct tlist *
tlist_new(struct region *reg, int rw, int rh, int flags)
{
	struct tlist *tl;

	tl = Malloc(sizeof(struct tlist));
	tlist_init(tl, rw, rh, flags);
	region_attach(reg, tl);
	return (tl);
}

void
tlist_init(struct tlist *tl, int rw, int rh, int flags)
{
	widget_init(&tl->wid, "tlist", &tlist_ops, rw, rh);
	WIDGET(tl)->flags |= WIDGET_CLIPPING;

	widget_map_color(tl, BACKGROUND_COLOR, "background", 120, 120, 120);
	widget_map_color(tl, TEXT_COLOR, "text", 240, 240, 240);
	widget_map_color(tl, LINE_COLOR, "line", 50, 50, 50);
	widget_map_color(tl, SELECTION_COLOR, "selection", 50, 50, 120);

	tl->flags = flags;
	tl->item_h = text_font_height(font) + 2;
	tl->nitems = 0;
	tl->nvisitems = 0;
	tl->dblclicked = NULL;
	tl->dbltimer = NULL;
	TAILQ_INIT(&tl->items);
	TAILQ_INIT(&tl->selitems);

	pthread_mutex_init(&tl->lock, &recursive_mutexattr);

	scrollbar_init(&tl->sbar, -1, -1, SCROLLBAR_VERT);
	WIDGET(&tl->sbar)->flags |= WIDGET_NO_FOCUS;

	event_new(tl, "window-mousemotion", tlist_mousemotion, NULL);
	event_new(tl, "window-mousebuttonup", tlist_mousebuttonup, NULL);
	event_new(tl, "window-mousebuttondown", tlist_mousebuttondown, NULL);
	event_new(tl, "window-keydown", tlist_keydown, NULL);
	event_new(tl, "attached", tlist_attached, NULL);
	event_new(tl, "widget-scaled", tlist_scaled, NULL);
}

void
tlist_destroy(void *p)
{
	struct tlist *tl = p;

	pthread_mutex_lock(&tl->lock);
	if (tl->dbltimer != NULL) {
		SDL_RemoveTimer(tl->dbltimer);
	}
	pthread_mutex_unlock(&tl->lock);

	tlist_clear_items(tl);
	pthread_mutex_destroy(&tl->lock);
	object_destroy(&tl->sbar);
	widget_destroy(tl);
}

static void
tlist_attached(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	
	WIDGET(&tl->sbar)->win = WIDGET(tl)->win;
}

static void
tlist_scaled(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct scrollbar *sb = &tl->sbar;

	WIDGET(sb)->x = WIDGET(tl)->x + WIDGET(tl)->w - DEFAULT_BUTTON_SIZE;
	WIDGET(sb)->y = WIDGET(tl)->y;
 	WIDGET(sb)->w = DEFAULT_BUTTON_SIZE;
	WIDGET(sb)->h = WIDGET(tl)->h;

	event_post(sb, "widget-scaled", "%i, %i", WIDGET(sb)->w, WIDGET(sb)->h);
}

void
tlist_draw(void *p)
{
	struct tlist *tl = p;
	struct tlist_item *it;
	int y = 0, i = 0;
	int offset, visitems = 0;

	if (WIDGET(tl)->w < MIN_WIDTH || WIDGET(tl)->h < MIN_HEIGHT)
		return;

	primitives.box(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h, -1,
	    WIDGET_COLOR(tl, BACKGROUND_COLOR));

	pthread_mutex_lock(&tl->lock);

	if (tl->flags & TLIST_POLL)
		event_post(tl, "tlist-poll", NULL);

	offset = widget_get_int(&tl->sbar, "value");

	TAILQ_FOREACH(it, &tl->items, items) {
		SDL_Surface *textsu = NULL;
		int x = 2;

		if (i++ < offset)
			continue;
		if (y > WIDGET(tl)->h - tl->item_h)
			break;

		if (it->selected) {			/* Selection mark */
			SDL_Rect rd;

			rd.x = x;
			rd.y = y;
			rd.w = WIDGET(tl)->w - WIDGET(&tl->sbar)->w;
			rd.h = tl->item_h;

			primitives.rect_filled(tl, &rd,
			    WIDGET_COLOR(tl, SELECTION_COLOR));
		}

		if (it->text != NULL) {
			textsu = text_render(NULL, -1,
			    WIDGET_COLOR(tl, TEXT_COLOR), it->text);
		}

		if (it->icon != NULL) {
			widget_blit(tl, it->icon, x, y);
		}
		
		if (it->haschilds) {
			int ts = tl->item_h/2;
			int ty = y + ts/2;

			primitives.rect_outlined(tl,
			    x+5, ty,
			    ts, ts,
			    WIDGET_COLOR(tl, LINE_COLOR));
			
			primitives.plus(tl,
			    !it->vischilds,
			    x+6, ty+1,
			    ts-2, ts-2,
			    WIDGET_COLOR(tl, LINE_COLOR));
		}

		x += tl->item_h + 4;
		if (textsu != NULL) {
			widget_blit(tl, textsu,
			    x,
			    y + tl->item_h/2 - textsu->h/2);
			SDL_FreeSurface(textsu);
		}

		y += tl->item_h;
		primitives.line(tl, 0, y, WIDGET(tl)->w, y,
		    WIDGET_COLOR(tl, LINE_COLOR));

		visitems++;
	}

	tl->nvisitems = visitems;
	if (tl->nitems > 0 && visitems > 0 && visitems < tl->nitems) {
		scrollbar_set_bar_size(&tl->sbar,
		    visitems*(WIDGET(&tl->sbar)->h - tl->sbar.button_size*2) /
		    tl->nitems);
	} else {
		scrollbar_set_bar_size(&tl->sbar, -1);		/* Full range */
	}

	scrollbar_draw(&tl->sbar);
	pthread_mutex_unlock(&tl->lock);
}

/* Adjust the scrollbar offset according to the number of visible items. */
static void
tlist_adjust_scrollbar(struct tlist *tl)
{
	struct widget_binding *maxb, *offsetb;
	int *max, *offset;
	int noffset;

	maxb = widget_binding_get_locked(&tl->sbar, "max", &max);
	offsetb = widget_binding_get_locked(&tl->sbar, "value", &offset);
	noffset = *offset;

	if (noffset > *max - tl->nvisitems)
		noffset = *max - tl->nvisitems;
	if (noffset < 0)
		noffset = 0;

	if (*offset != noffset) {
		*offset = noffset;
		widget_binding_modified(offsetb);
	}
	widget_binding_unlock(offsetb);
	widget_binding_unlock(maxb);
}

static void
tlist_free_item(struct tlist_item *it)
{
	if (it->icon != NULL)
		SDL_FreeSurface(it->icon);
	free(it);
}

void
tlist_remove_item(struct tlist *tl, struct tlist_item *it)
{
	struct widget_binding *offsetb;
	int *offset;
	int nitems;

	/* Destroy the item. */
	pthread_mutex_lock(&tl->lock);
	TAILQ_REMOVE(&tl->items, it, items);
	nitems = --tl->nitems;
	pthread_mutex_unlock(&tl->lock);
	tlist_free_item(it);

	/* Update the scrollbar range and offset accordingly. */
	widget_set_int(&tl->sbar, "max", nitems);
	offsetb = widget_binding_get_locked(&tl->sbar, "value", &offset);
	if (*offset > nitems) {
		*offset = nitems;
	} else if (nitems > 0 && *offset < nitems) {		/* XXX ugly */
		*offset = 0;
	}
	widget_binding_unlock(offsetb);
}

/* Set the position of the scrollbar. */
void
tlist_scroll(struct tlist *tl, int pos)
{
	pthread_mutex_lock(&tl->lock);
	widget_set_int(&tl->sbar, "value", pos);
	pthread_mutex_unlock(&tl->lock);
}

/* Clear the items on the list. */
void
tlist_clear_items(struct tlist *tl)
{
	struct tlist_item *it, *nit;
	
	pthread_mutex_lock(&tl->lock);

	if (tl->flags & TLIST_POLL) {
		/* Clear the saved selection list. */
		for (it = TAILQ_FIRST(&tl->selitems);
		     it != TAILQ_END(&tl->selitems);
		     it = nit) {
			nit = TAILQ_NEXT(it, selitems);
			tlist_free_item(it);
		}
		TAILQ_INIT(&tl->selitems);
	}

	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if ((tl->flags & TLIST_POLL) &&
		    (it->selected || it->haschilds)) {
			TAILQ_INSERT_HEAD(&tl->selitems, it, selitems);
		} else {
			tlist_free_item(it);
		}
	}
	TAILQ_INIT(&tl->items);
	tl->nitems = 0;
	
	/* Preserve the offset value, for polling. */
	widget_set_int(&tl->sbar, "max", 0);

	pthread_mutex_unlock(&tl->lock);
}

static __inline__ int
tlist_item_compare(struct tlist_item *it1, struct tlist_item *it2)
{
	return (it1->p1 == it2->p1);
}

/* Restore previous item selection state. */
void
tlist_restore_selections(struct tlist *tl)
{
	struct tlist_item *sit, *cit;

	TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		TAILQ_FOREACH(cit, &tl->items, items) {
			if (tlist_item_compare(sit, cit)) {
				cit->selected = sit->selected;
				cit->vischilds = sit->vischilds;
			}
		}
	}
	tlist_adjust_scrollbar(tl);
}

/*
 * See if the child items of an item are supposed to be visible; called from
 * polling routines when displaying trees.
 */
int
tlist_visible_childs(struct tlist *tl, struct tlist_item *cit)
{
	struct tlist_item *sit;

	TAILQ_FOREACH(sit, &tl->selitems, selitems) {
		if (tlist_item_compare(sit, cit))
			break;
	}
	if (sit == NULL) 
		return (0);				/* Default */

	return (sit->vischilds);
}

static struct tlist_item *
tlist_alloc_item(struct tlist *tl, SDL_Surface *icon, char *text, void *p1)
{
	struct tlist_item *it;

	it = Malloc(sizeof(struct tlist_item));
	it->icon = NULL;
	it->selected = 0;
	it->vischilds = 0;
	it->haschilds = 0;

	strlcpy(it->text, text, sizeof(it->text));
	it->text_len = strlen(text);
	it->p1 = p1;
	tlist_set_item_icon(tl, it, icon);			/* Square */
	return (it);
}

/* Add an item to the list. */
struct tlist_item *
tlist_insert_item(struct tlist *tl, SDL_Surface *icon, char *text, void *p1)
{
	struct tlist_item *it;

	it = tlist_alloc_item(tl, icon, text, p1);

	pthread_mutex_lock(&tl->lock);
	TAILQ_INSERT_TAIL(&tl->items, it, items);
	widget_set_int(&tl->sbar, "max", ++tl->nitems);
	pthread_mutex_unlock(&tl->lock);

	event_post(tl, "tlist-inserted-item", "%p", it);
	return (it);
}

struct tlist_item *
tlist_insert_item_head(struct tlist *tl, SDL_Surface *icon, char *text,
    void *p1)
{
	struct tlist_item *it;

	it = tlist_alloc_item(tl, icon, text, p1);

	pthread_mutex_lock(&tl->lock);
	TAILQ_INSERT_HEAD(&tl->items, it, items);
	widget_set_int(&tl->sbar, "max", ++tl->nitems);
	pthread_mutex_unlock(&tl->lock);

	event_post(tl, "tlist-inserted-item", "%p", it);
	return (it);
}

/* Set the selection flag on an item. */
int
tlist_select(struct tlist *tl, struct tlist_item *it)
{
	int old;

	pthread_mutex_lock(&tl->lock);
	old = it->selected;
	it->selected++;
	pthread_mutex_unlock(&tl->lock);

	return (old);
}

/* Clear the selection flag on an item. */
int
tlist_unselect(struct tlist *tl, struct tlist_item *it)
{
	int old;

	pthread_mutex_lock(&tl->lock);
	old = it->selected;
	it->selected = 0;
	pthread_mutex_unlock(&tl->lock);

	return (old);
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
tlist_mousemotion(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	
	event_forward(&tl->sbar, "window-mousemotion", argc, argv);
	tlist_adjust_scrollbar(tl);
}

static void
tlist_mousebuttonup(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	
	event_forward(&tl->sbar, "window-mousebuttonup", argc, argv);
}

static Uint32
tlist_dblclick_expire(Uint32 i, void *arg)
{
	struct tlist *tl = arg;

	pthread_mutex_lock(&tl->lock);
	dprintf("%s: double click expired\n", OBJECT(tl)->name);
	tl->dblclicked = NULL;
	pthread_mutex_unlock(&tl->lock);
	return (0);
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
	
	WIDGET_FOCUS(tl);

	if (x > WIDGET(tl)->w - WIDGET(&tl->sbar)->w) {	/* Scrollbar click? */
		event_forward(&tl->sbar, "window-mousebuttondown", argc, argv);
		tlist_adjust_scrollbar(tl);
		return;
	}
	
	if (button != SELECTION_MOUSE_BUTTON) 		/* Selection button? */
		return;
	
	pthread_mutex_lock(&tl->lock);

	tind = (widget_get_int(&tl->sbar, "value") + y/tl->item_h) + 1;
	if ((ti = tlist_item_index(tl, tind)) == NULL)
		goto out;

	if (ti->haschilds &&
	    x >= 5 && x <= 5+tl->item_h/2) {		/* Tree visible flag */
		ti->vischilds = !ti->vischilds;
		goto out;
	}

	if (tl->flags & TLIST_MULTI) {			/* Multi selections */
		if (SDL_GetModState() & KMOD_SHIFT) {
			struct tlist_item *oitem;
			int oind = -1, i = 0, nitems = 0;

			TAILQ_FOREACH(oitem, &tl->items, items) {
				if (oitem->selected)
					oind = i;
				i++;
				nitems++;
			}
			if (oind == -1)
				goto out;
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
		} else {
			if ((tl->flags & TLIST_MULTI_STICKY) ||
			    (SDL_GetModState() & KMOD_CTRL)) {
			    	ti->selected = !ti->selected;
			} else {
				tlist_unselect_all(tl);
				ti->selected++;
			}
		}
	} else {					  /* Single selection */
		tlist_unselect_all(tl);
		ti->selected++;

		if (tl->flags & TLIST_DBLCLICK) {
			if (tl->dblclicked == ti) {
				dprintf("%s: double-click effect\n",
				    OBJECT(tl)->name);
				if (tl->dbltimer != NULL)
					SDL_RemoveTimer(tl->dbltimer);
				event_post(tl, "tlist-dblclick", "%p",
				    tl->dblclicked);
				tl->dblclicked = NULL;
				tl->dbltimer = NULL;
			} else {
				dprintf("%s: double-click start\n",
				    OBJECT(tl)->name);
				if (tl->dbltimer != NULL)
					SDL_RemoveTimer(tl->dbltimer);
				tl->dblclicked = ti;
				tl->dbltimer = SDL_AddTimer(DBLCLICK_DELAY,
				    tlist_dblclick_expire, tl);
			}
		}
	}
	event_post(tl, "tlist-changed", "%p, %i", ti, 1);
out:
	pthread_mutex_unlock(&tl->lock);
}

static void
tlist_keydown(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it, *pit;
	struct widget_binding *offsetb;
	int keysym = argv[1].i;
	int sel;
	int *offset;

	offsetb = widget_binding_get_locked(&tl->sbar, "value", &offset);

	pthread_mutex_lock(&tl->lock);
	switch (keysym) {
	case SDLK_UP:
		sel = 0;
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				pit = TAILQ_PREV(it, tlist_itemq, items);
				if (pit != NULL) {
					/* Unselect current */
					it->selected = 0;
					event_post(tl, "tlist-changed",
					    "%p, %i", it, 0);

					/* Select previous */
					pit->selected++;
					event_post(tl, "tlist-changed",
					    "%p, %i", pit, 1);
				
					/* Scroll */
					if (--(*offset) < 0) {
						*offset = 0;
					}
					break;
				}
			}
			sel++;
		}
		break;
	case SDLK_DOWN:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				pit = TAILQ_NEXT(it, items);
				if (pit != NULL) {
					/* Unselect current */
					it->selected = 0;
					event_post(tl, "tlist-changed",
					    "%p, %i", it, 0);
					
					/* Select next */
					pit->selected++;
					event_post(tl, "tlist-changed",
					    "%p, %i", pit, 1);
					
					/* Scroll */
					if (++(*offset) >
					    tl->nitems - tl->nvisitems) {
						*offset = tl->nitems -
						    tl->nvisitems;
					}
					break;
				}
			}
		}
		break;
	case SDLK_PAGEUP:
		if ((*offset -= PAGE_INCREMENT) < 0)
			*offset = 0;
		widget_binding_modified(offsetb);
		break;
	case SDLK_PAGEDOWN:
		if ((*offset += PAGE_INCREMENT) > tl->nitems - tl->nvisitems)
			*offset = tl->nitems - tl->nvisitems;
		widget_binding_modified(offsetb);
		break;
	default:
		break;
	}
	widget_binding_unlock(offsetb);
	pthread_mutex_unlock(&tl->lock);
}

/* Return the item at the given index. */
struct tlist_item *
tlist_item_index(struct tlist *tl, int index)
{
	struct tlist_item *it;
	int i = 0;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (++i == index) {
			pthread_mutex_unlock(&tl->lock);
			return (it);
		}
	}
	pthread_mutex_unlock(&tl->lock);
	return (NULL);
}

/* Return the first selected item. */
struct tlist_item *
tlist_item_selected(struct tlist *tl)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			pthread_mutex_unlock(&tl->lock);
			return (it);
		}
	}
	pthread_mutex_unlock(&tl->lock);
	return (NULL);
}

/* Return the first item matching a text string. */
struct tlist_item *
tlist_item_text(struct tlist *tl, char *text)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (strcmp(it->text, text) == 0) {
			pthread_mutex_unlock(&tl->lock);
			return (it);
		}
	}
	pthread_mutex_unlock(&tl->lock);
	return (NULL);
}

/* If the tlist is attached, its parent window must be locked. */
void
tlist_set_item_height(struct tlist *tl, int ih)
{
	struct tlist_item *it;

	tl->item_h = ih;

	pthread_mutex_lock(&tl->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->icon != NULL) {
			SDL_Surface *nicon;

			nicon = view_scale_surface(it->icon,
			    tl->item_h, tl->item_h);		/* Square */
			SDL_FreeSurface(it->icon);
			it->icon = nicon;
		}
	}
	pthread_mutex_unlock(&tl->lock);
}

/* Update an item's icon. */
void
tlist_set_item_icon(struct tlist *tl, struct tlist_item *it, SDL_Surface *icon)
{
	if (it->icon != NULL)
		SDL_FreeSurface(it->icon);

	if (icon != NULL) {
		it->icon = view_scale_surface(icon,
		    tl->item_h, tl->item_h);			/* Square */
	} else {
		it->icon = NULL;
	}
}

