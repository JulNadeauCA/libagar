/*	$Csoft: plist.c,v 1.2 2004/05/24 00:33:35 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include "plist.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <string.h>

static struct widget_ops plist_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		plist_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	plist_draw,
	plist_scale
};

enum {
	TEXT_COLOR,
	BG_COLOR,
	LINE_COLOR,
	SELECTION_COLOR
};
enum {
	PAGE_INCREMENT = 4
};

static void plist_mousebuttondown(int, union evarg *);
static void plist_keydown(int, union evarg *);
static void plist_keyup(int, union evarg *);
static void plist_scrolled(int, union evarg *);
static void plist_select_item(struct plist *, struct plist_item *);
static void plist_unselect_item(struct plist *, struct plist_item *);

struct plist *
plist_new(void *parent, int flags)
{
	struct plist *pl;

	pl = Malloc(sizeof(struct plist), M_OBJECT);
	plist_init(pl, flags);
	object_attach(parent, pl);
	return (pl);
}

/* Displace the selection or scroll in response to keyboard events. */
static void
key_tick(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	struct plist_item *it, *pit;
	struct widget_binding *offsetb;
	int *offset;

	/* ... */

	pl->keymoved++;
	event_resched(pl, "key-tick", kbd_repeat);
}

static void
dblclick_expire(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;

	pthread_mutex_lock(&pl->lock);
	pl->dblclicked = 0;
	pthread_mutex_unlock(&pl->lock);
}

static void
lost_focus(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;

	event_cancel(pl, "key-tick");
	event_cancel(pl, "dblclick-expire");
	pl->keymoved = 0;
}

void
plist_init(struct plist *pl, int flags)
{
	widget_init(pl, "plist", &plist_ops,
	    WIDGET_FOCUSABLE|WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL);
	widget_bind(pl, "selected", WIDGET_POINTER, &pl->selected);
	widget_map_color(pl, BG_COLOR, "background", 120, 120, 120, 255);
	widget_map_color(pl, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(pl, LINE_COLOR, "line", 50, 50, 50, 255);
	widget_map_color(pl, SELECTION_COLOR, "selection", 50, 50, 120, 255);
	
	pl->flags = flags;
	pl->item_h = text_font_height(NULL)+2;
	pl->dblclicked = 0;
	pl->keymoved = 0;
	pl->items = Malloc(16 * sizeof(struct plist_item), M_WIDGET);
	pl->nitems = 0;
	pl->maxitems = 16;
	pl->nvisitems = 0;
	pl->sbar = scrollbar_new(pl, SCROLLBAR_VERT);
	pthread_mutex_init(&pl->lock, &recursive_mutexattr);
	plist_prescale(pl, "XXXXXXXXXXXXXXX", 4);

	event_new(pl->sbar, "scrollbar-changed", plist_scrolled, "%p", pl);
	event_new(pl, "window-mousebuttondown", plist_mousebuttondown, NULL);
	event_new(pl, "window-keydown", plist_keydown, NULL);
	event_new(pl, "window-keyup", plist_keyup, NULL);
	event_new(pl, "key-tick", key_tick, NULL);
	event_new(pl, "dblclick-expire", dblclick_expire, NULL);
	event_new(pl, "widget-lostfocus", lost_focus, NULL);
	event_new(pl, "widget-hidden", lost_focus, NULL);
}

void
plist_prescale(struct plist *pl, const char *text, int nitems)
{
	text_prescale(text, &pl->prew, NULL);
	pl->prew += pl->item_h + 5;
	pl->preh = (pl->item_h+2)*nitems;
}

void
plist_destroy(void *p)
{
	struct plist *pl = p;

	Free(pl->items, M_WIDGET);
	pthread_mutex_destroy(&pl->lock);
	widget_destroy(pl);
}

void
plist_scale(void *p, int w, int h)
{
	struct plist *pl = p;

	if (w == -1 && h == -1) {
		WIDGET(pl)->w = pl->prew;
		WIDGET(pl)->h = pl->preh;
	}
	WIDGET(pl->sbar)->x = WIDGET(pl)->w - pl->sbar->button_size;
	WIDGET(pl->sbar)->y = 0;
	widget_scale(pl->sbar, pl->sbar->button_size, WIDGET(pl)->h);
}

void
plist_draw(void *p)
{
	struct plist *pl = p;
	struct plist_item *it;
	int y = 0, i = 0;
	int offset;

	primitives.box(pl, 0, 0, WIDGET(pl)->w, WIDGET(pl)->h, -1, BG_COLOR);

	pthread_mutex_lock(&pl->lock);
	event_post(NULL, pl, "plist-poll", NULL);
	offset = widget_get_int(pl->sbar, "value");
	for (i = offset;
	     i < pl->nitems && y < WIDGET(pl)->h-pl->item_h;
	     i++) {
		struct plist_item *it = &pl->items[i];
		int ts = pl->item_h/2 + 1;
		int x = 2;

		if (pl->selitems[i]) {
			primitives.rect_filled(pl, x, y,
			    WIDGET(pl)->w - WIDGET(pl->sbar)->w, pl->item_h,
			    SELECTION_COLOR);
		}

		if (it->icon != NULL) {
			widget_blit(pl, it->icon, x, y);
		}
		x += pl->item_h+5;

		if (it->label == NULL) {
			it->label = text_render(NULL, -1,
			    WIDGET_COLOR(pl, TEXT_COLOR), it->text);
		}
		widget_blit(pl, it->label, x,
		    y + pl->item_h/2 - it->label->h/2);
		y += pl->item_h;

		primitives.line(pl, 0, y, WIDGET(pl)->w, y, LINE_COLOR);
	}

	pl->nvisitems = i-offset;
	
	if (pl->nitems > 0 && pl->nvisitems > 0 &&
	    pl->nvisitems < pl->nitems) {
		scrollbar_set_bar_size(pl->sbar, pl->nvisitems *
		    (WIDGET(pl->sbar)->h - pl->sbar->button_size*2) /
		    pl->nitems);
	} else {
		scrollbar_set_bar_size(pl->sbar, -1);
	}
	pthread_mutex_unlock(&pl->lock);
}

/* Adjust the scrollbar offset according to the number of visible items. */
static __inline__ void
plist_adjust_scrollbar(struct plist *pl)
{
	struct widget_binding *maxb, *offsetb;
	int *max, *offset;
	int noffset;

	maxb = widget_get_binding(pl->sbar, "max", &max);
	offsetb = widget_get_binding(pl->sbar, "value", &offset);
	noffset = *offset;

	if (noffset > *max - pl->nvisitems) {
		noffset = *max - pl->nvisitems;
	}
	if (noffset < 0)
		noffset = 0;

	if (*offset != noffset) {
		*offset = noffset;
		widget_binding_modified(offsetb);
	}
	widget_binding_unlock(offsetb);
	widget_binding_unlock(maxb);
}

void
plist_clear(struct plist *pl)
{
	pthread_mutex_lock(&pl->lock);
	pl->nitems = 0;
	pl->nvisitems = 0;
	pthread_mutex_unlock(&pl->lock);
}

struct plist_item *
plist_insert(struct plist *pl, SDL_Surface *icon, const char *text, void *p)
{
	struct plist_item *it;
	
	pthread_mutex_lock(&pl->lock);
	if ((pl->nitems+1) >= pl->maxitems) {
		pl->maxitems += 8;
		pl->items = Realloc(pl->items, pl->maxitems *
		                               sizeof(struct plist_item));
		pl->selitems = Realloc(pl->selitems, pl->maxitems*sizeof(int));
	}
	it = &pl->items[pl->nitems];
	pl->selitems[pl->nitems] = 0;
	pl->nitems++;

	it->icon = icon;
	it->p = p;
	it->label = NULL;
	strlcpy(it->text, text, sizeof(it->text));

	widget_set_int(pl->sbar, "max", pl->nitems);
	pthread_mutex_unlock(&pl->lock);
	return (it);
}

static void
plist_select_item(struct plist *pl, struct plist_item *it)
{
	struct widget_binding *selectedb;
	void **selected;

	selectedb = widget_get_binding(pl, "selected", &selected);
	*selected = it->p;
	event_post(NULL, pl, "plist-changed", "%p, %i", it, 1);
	event_post(NULL, pl, "plist-selected", "%p", it);
	widget_binding_modified(selectedb);
	widget_binding_unlock(selectedb);
}

static void
plist_unselect_item(struct plist *pl, struct plist_item *it)
{
	struct widget_binding *selectedb;
	void **selected;

	selectedb = widget_get_binding(pl, "selected", &selected);
	*selected = NULL;
	event_post(NULL, pl, "plist-changed", "%p, %i", it, 0);
	widget_binding_modified(selectedb);
	widget_binding_unlock(selectedb);
}

static void
plist_mousebuttondown(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct plist_item *ti;
	
	widget_focus(pl);
	if (button != 1)
		return;
	
	pthread_mutex_lock(&pl->lock);
	pthread_mutex_unlock(&pl->lock);
}

static void
plist_keydown(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&pl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		event_schedule(NULL, pl, kbd_delay, "key-tick", "%i", keysym);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&pl->lock);
}

static void
plist_keyup(int argc, union evarg *argv)
{
	struct plist *pl = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&pl->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		if (pl->keymoved == 0) {
			event_post(NULL, pl, "key-tick", "%i", keysym);
		}
		event_cancel(pl, "key-tick");
		pl->keymoved = 0;
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&pl->lock);
}

static void
plist_scrolled(int argc, union evarg *argv)
{
	struct plist *pl = argv[1].p;

	plist_adjust_scrollbar(pl);
}

/* Set the height to use for item display. */
void
plist_set_item_height(struct plist *pl, int ih)
{
	struct plist_item *it;

	pthread_mutex_lock(&pl->lock);
	pl->item_h = ih;
	pthread_mutex_unlock(&pl->lock);
}

/* Change the icon associated with an item. */
void
plist_set_icon(struct plist *pl, struct plist_item *it, SDL_Surface *nicon)
{
	pthread_mutex_lock(&pl->lock);
	it->icon = nicon;
	pthread_mutex_unlock(&pl->lock);
}

