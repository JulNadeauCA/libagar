/*	$Csoft: tlist.c,v 1.5 2002/09/09 01:24:40 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "tlist.h"

static struct widget_ops tlist_ops = {
	{
		tlist_destroy,	/* destroy */
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
	SELECTION_MOUSE_BUTTON = 1
};

static void	tlist_mouse_button(int, union evarg *);
static void	tlist_mouse_motion(int, union evarg *);
static void	tlist_keydown(int, union evarg *);
static void	tlist_scaled(int, union evarg *);
static void	tlist_attached(int, union evarg *);
static void	tlist_free_item(struct tlist_item *);

struct tlist *
tlist_new(struct region *reg, int rw, int rh, int flags)
{
	struct tlist *tl;

	tl = emalloc(sizeof(struct tlist));
	tlist_init(tl, rw, rh, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, tl);
	pthread_mutex_unlock(&reg->win->lock);

	return (tl);
}

void
tlist_init(struct tlist *tl, int rw, int rh, int flags)
{
	widget_init(&tl->wid, "tlist", "widget", &tlist_ops, rw, rh);

	widget_map_color(tl, TEXT_COLOR, "tlist-text",
	    240, 240, 240);
	widget_map_color(tl, BACKGROUND_COLOR, "tlist-background",
	    120, 120, 120);
	widget_map_color(tl, LINE_COLOR, "tlist-line",
	    50, 50, 50);
	widget_map_color(tl, SELECTION_COLOR, "tlist-selection",
	    50, 50, 120);

	tl->flags = flags;
	tl->xspacing = 6;
	tl->yspacing = 3;
	tl->item_h = 16;
	tl->ops.update = NULL;
	tl->nitems = 0;
	TAILQ_INIT(&tl->items);
	pthread_mutex_init(&tl->items_lock, NULL);
	
	tl->vbar = emalloc(sizeof(struct scrollbar));
	scrollbar_init(tl->vbar, -1, -1, tl->item_h, SCROLLBAR_VERTICAL);

	event_new(tl, "window-mousemotion", 0, tlist_mouse_motion, NULL);
	event_new(tl, "window-mousebuttondown", 0, tlist_mouse_button, NULL);
	event_new(tl, "window-keydown", 0, tlist_keydown, NULL);
	event_new(tl, "attached", 0, tlist_attached, NULL);
	event_new(tl, "widget-scaled", 0, tlist_scaled, NULL);
}

void
tlist_destroy(void *p)
{
	struct tlist *tl = p;
	struct tlist_item *it, *nextit;

	tlist_clear_items(tl);
	pthread_mutex_destroy(&tl->items_lock);
	
	object_destroy(tl->vbar);

	widget_destroy(tl);
}

static void
tlist_attached(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	
	WIDGET(tl->vbar)->win = WIDGET(tl)->win;
}

static void
tlist_scaled(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct scrollbar *sb = tl->vbar;
	int w = argv[1].i;
	int h = argv[2].i;

	WIDGET(sb)->x = WIDGET(tl)->x + WIDGET(tl)->w - 20;
	WIDGET(sb)->y = WIDGET(tl)->y;
	WIDGET(sb)->w = 20;
	WIDGET(sb)->h = WIDGET(tl)->h;
}

void
tlist_draw(void *p)
{
	SDL_Rect rd;
	struct tlist *tl = p;
	struct tlist_item *it;
	struct scrollbar *sb = tl->vbar;
	int item_h;
	int y, i = 0;

	primitives.box(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h, -1,
	    WIDGET_COLOR(tl, BACKGROUND_COLOR));

	if (tl->ops.update != NULL) {		/* XXX */
		tl->ops.update(tl);
	}

	pthread_mutex_lock(&tl->items_lock);

	y = sb->range.soft_start;
	TAILQ_FOREACH(it, &tl->items, items) {
		SDL_Surface *su;
		int x = 2;

		if (i++ < sb->range.start) {
			continue;
		}

		if (y > WIDGET(tl)->h - it->icon_w) {
			goto out;
		}

		if (it->selected) {
			WIDGET_FILL(tl, x, y,
			    WIDGET(tl)->w - WIDGET(sb)->w, tl->item_h,
			    WIDGET_COLOR(tl, SELECTION_COLOR));
		}

		su = text_render(NULL, -1,
		    WIDGET_COLOR(tl, TEXT_COLOR), it->text);
	
		if (it->icon != NULL) {
			WIDGET_DRAW(tl, it->icon, x, y);
		}
		x += it->icon_w + 4;

		WIDGET_DRAW(tl, su, x, y);
		y += tl->item_h;
		primitives.line(tl, 0, y, WIDGET(tl)->w, y,
		    WIDGET_COLOR(tl, LINE_COLOR));

		SDL_FreeSurface(su);
	}
out:
	scrollbar_draw(sb);
	pthread_mutex_unlock(&tl->items_lock);
}

static void
tlist_free_item(struct tlist_item *it)
{
	if (it->icon != NULL) {
		view_unused_surface(it->icon);
	}
	free(it->text);
	free(it);
}

void
tlist_remove_item(struct tlist_item *it)
{
	struct tlist *tl = it->tl_bp;
	struct scrollbar *sb = tl->vbar;

	pthread_mutex_lock(&tl->items_lock);
	TAILQ_REMOVE(&tl->items, it, items);
	scrollbar_set_range(sb, --tl->nitems);
	pthread_mutex_unlock(&tl->items_lock);

	tlist_free_item(it);
}

void
tlist_clear_items(struct tlist *tl)
{
	struct tlist_item *it, *nit;
	struct scrollbar *sb = tl->vbar;
	
	pthread_mutex_lock(&tl->items_lock);

	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		tlist_free_item(it);
	}
	TAILQ_INIT(&tl->items);
	tl->nitems = 0;
	
	scrollbar_set_range(sb, 0);
	
	pthread_mutex_unlock(&tl->items_lock);
}

struct tlist_item *
tlist_insert_item(struct tlist *tl, SDL_Surface *icon, char *text, void *p1)
{
	struct tlist_item *it;
	struct scrollbar *sb = tl->vbar;

	it = emalloc(sizeof(struct tlist_item));
	it->icon_w = 16;
	it->icon_h = 16;
	it->selected = 0;
	if (icon != NULL) {
		it->icon = view_scale_surface(icon, it->icon_w, it->icon_h);
	} else {
		it->icon = NULL;
	}
	it->text = strdup(text);
	it->text_len = strlen(text);
	it->p1 = p1;
	it->tl_bp = tl;

	pthread_mutex_lock(&tl->items_lock);
	TAILQ_INSERT_HEAD(&tl->items, it, items);
	scrollbar_set_range(sb, ++tl->nitems);
	pthread_mutex_unlock(&tl->items_lock);

	event_post(tl, "tlist-inserted-item", "%p", it);

	return (it);
}

/*
 * Unset the selection flag on all items.
 * Item list must not be locked by the caller thread.
 */
void
tlist_unselect_items(struct tlist *tl)
{
	struct tlist_item *it;

	pthread_mutex_lock(&tl->items_lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		it->selected = 0;
	}
	pthread_mutex_unlock(&tl->items_lock);
}

static void
tlist_mouse_motion(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct scrollbar *sb = tl->vbar;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	Uint8 ms;
	
	if (x > WIDGET(tl)->w - WIDGET(tl->vbar)->w) {
		event_forward(tl->vbar, "window-mousemotion", argc, argv);
		return;
	}

	ms = SDL_GetMouseState(NULL, NULL);
	if (ms & SDL_BUTTON_RMASK) {
		if (yrel < 0 && (sb->range.soft_start += yrel) < 0) {
			sb->range.soft_start = tl->item_h;
			if (++sb->range.start > tl->nitems) {
				sb->range.start = tl->nitems;
			}
		}
		if (yrel > 0 && sb->range.start > 0 &&
		    (sb->range.soft_start += yrel) > tl->item_h) {
			sb->range.soft_start = 0;
			if (--sb->range.start < 0) {
				sb->range.start = 0;
			}
		}
	}
}

static void
tlist_mouse_button(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct scrollbar *sb = tl->vbar;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct tlist_item *ti;

	if (x > WIDGET(tl)->w - WIDGET(sb)->w) {
		event_forward(sb, "window-mousebuttondown", argc, argv);
		return;
	}
	
	WIDGET_FOCUS(tl);
	if (button != SELECTION_MOUSE_BUTTON) {
		return;
	}
	
	pthread_mutex_lock(&tl->items_lock);
	ti = _tlist_item_index(tl,
	    (sb->range.start + (y - sb->range.soft_start) / tl->item_h), 0);
	if (ti != NULL) {
		if (tl->flags & TLIST_MULTI_STICKY ||
		   ((tl->flags & TLIST_MULTI) &&
		    (SDL_GetModState() & KMOD_CTRL))) {
			if (ti->selected) {
				ti->selected = 0;
			} else {
				ti->selected++;
			}
		} else {
			tlist_unselect_items(tl);
			ti->selected++;
		}
		event_post(tl, "tlist-changed", "%p, %i", ti, 1);
	}
	pthread_mutex_unlock(&tl->items_lock);
}

static void
tlist_keydown(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct scrollbar *sb = tl->vbar;
	struct tlist_item *it, *pit;
	int keysym = argv[1].i;
	int sel;

	pthread_mutex_lock(&tl->items_lock);
	switch (keysym) {
	case SDLK_UP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				dprintf("to previous\n");
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
					break;
				} else {
					dprintf("no previous item\n");
				}
			}
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
					break;
				}
			}
		}
		break;
	case SDLK_LEFT:
		if (--sb->range.start < 0) {
			sb->range.start = 0;
		}
		break;
	case SDLK_RIGHT:
		if (++sb->range.start > tl->nitems) {
			sb->range.start = tl->nitems;
		}
		break;
	case SDLK_PAGEUP:
		if ((sb->range.start -= 4) < 0) {
			sb->range.start = 0;
		}
		break;
	case SDLK_PAGEDOWN:
		if ((sb->range.start += 4) > tl->nitems) {
			sb->range.start = tl->nitems;
		}
		break;
	default:
	}
	
	pthread_mutex_unlock(&tl->items_lock);
}

/*
 * Return the item at the given index.
 * Item list must not be locked by the caller thread.
 */
struct tlist_item *
_tlist_item_index(struct tlist *tl, int index, int mustlock)
{
	struct tlist_item *it;
	int i = 0;

	if (mustlock) pthread_mutex_lock(&tl->items_lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (++i == index) {
			pthread_mutex_unlock(&tl->items_lock);
			return (it);
		}
	}
	if (mustlock) pthread_mutex_unlock(&tl->items_lock);
	return (NULL);
}

/*
 * Return the first item matching a text string.
 * Item list must not be locked by the caller thread.
 */
struct tlist_item *
_tlist_item_text(struct tlist *tl, char *text, int mustlock)
{
	struct tlist_item *it;
	int i;

	if (mustlock) pthread_mutex_lock(&tl->items_lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (strncmp(it->text, text, it->text_len) == 0) {
			pthread_mutex_unlock(&tl->items_lock);
			return (it);
		}
	}
	if (mustlock) pthread_mutex_unlock(&tl->items_lock);
	return (NULL);
}

