/*	$Csoft: tlist.c,v 1.4 2002/09/07 04:57:53 vedge Exp $	*/

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
	NULL		/* animate */
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
	tl->offs.start = 0;
	tl->offs.soft_start = 0;
	tl->offs.sel = -1;
	tl->nitems = 0;
	TAILQ_INIT(&tl->items);
	pthread_mutex_init(&tl->items_lock, NULL);

	event_new(tl, "window-mousemotion", 0, tlist_mouse_motion, NULL);
	event_new(tl, "window-mousebuttondown", 0, tlist_mouse_button, NULL);
	event_new(tl, "window-keydown", 0, tlist_keydown, NULL);
	event_new(tl, "attached", 0, tlist_attached, NULL);

	tl->vbar = emalloc(sizeof(struct scrollbar));
	scrollbar_init(tl->vbar, -1, -1, SCROLLBAR_VERTICAL);
	event_new(tl, "widget-scaled", 0, tlist_scaled, NULL);
}

void
tlist_destroy(void *p)
{
	struct tlist *tl = p;
	struct tlist_item *it, *nextit;

	for (it = TAILQ_FIRST(&tl->items);
	     it != TAILQ_END(&tl->items);
	     it = nextit) {
		nextit = TAILQ_NEXT(it, items);
		free(it);
	}

	object_destroy(tl->vbar);
	pthread_mutex_destroy(&tl->items_lock);
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
	int w = argv[1].i;
	int h = argv[2].i;
	struct scrollbar *sb = tl->vbar;

	WIDGET(sb)->x = WIDGET(tl)->x + w - 19;
	WIDGET(sb)->y = WIDGET(tl)->y;
	WIDGET(sb)->w = 20;
	WIDGET(sb)->h = h - 7;
}

void
tlist_draw(void *p)
{
	SDL_Rect rd;
	struct tlist *tl = p;
	struct tlist_item *it;
	int y, i = 0;

	primitives.box(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h, -1,
	    WIDGET_COLOR(tl, BACKGROUND_COLOR));

	if (tl->ops.update != NULL) {		/* XXX */
		tl->ops.update(tl);
	}

	pthread_mutex_lock(&tl->items_lock);

	y = tl->offs.soft_start;
	TAILQ_FOREACH(it, &tl->items, items) {
		int x = 2;
		SDL_Surface *su;

		if (i++ < tl->offs.start) {
			continue;
		}

		if (y > WIDGET(tl)->h - it->icon_w) {
			goto out;
		}

		if (i - 1 == tl->offs.sel) {
			WIDGET_FILL(tl, x, y,
			    WIDGET(tl)->w - WIDGET(tl->vbar)->w,
			    tl->item_h,
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
	pthread_mutex_unlock(&tl->items_lock);

	scrollbar_draw(tl->vbar);
}

void
tlist_remove_item(struct tlist_item *it)
{
	struct tlist *tl = it->tl_bp;

	pthread_mutex_lock(&tl->items_lock);
	TAILQ_REMOVE(&tl->items, it, items);
	scrollbar_set_range(tl->vbar, 0, --tl->nitems);
	pthread_mutex_unlock(&tl->items_lock);

	free(it->text);
	free(it);
}

void
tlist_clear_items(struct tlist *tl)
{
	struct tlist_item *it;
	
	pthread_mutex_lock(&tl->items_lock);

	TAILQ_FOREACH(it, &tl->items, items) {
		free(it->text);
		free(it);
	}
	TAILQ_INIT(&tl->items);
	tl->nitems = 0;

	pthread_mutex_unlock(&tl->items_lock);
	
	scrollbar_set_range(tl->vbar, 0, 0);
}

struct tlist_item *
tlist_insert_item(struct tlist *tl, SDL_Surface *icon, char *text, void *p1)
{
	struct tlist_item *it;

	it = emalloc(sizeof(struct tlist_item));
	it->icon_w = 16;
	it->icon_h = 16;
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
	scrollbar_set_range(tl->vbar, 0, ++tl->nitems);
	pthread_mutex_unlock(&tl->items_lock);

	event_post(tl, "tlist-inserted-item", "%p", it);

	return (it);
}

static void
tlist_mouse_motion(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	Uint8 ms;
	
	if (x > WIDGET(tl)->w - WIDGET(tl->vbar)->w) {
		event_post(tl->vbar, "window-mousemotion", "%i, %i, %i, %i",
		    x, y, xrel, yrel);
		return;
	}

	ms = SDL_GetMouseState(NULL, NULL);
	if (ms & SDL_BUTTON_RMASK) {
		if (yrel < 0 && (tl->offs.soft_start += yrel) < 0) {
			tl->offs.soft_start = tl->item_h;
			if (++tl->offs.start > tl->nitems) {
				tl->offs.start = tl->nitems;
			}
		}
		if (yrel > 0 && tl->offs.start > 0 &&
		    (tl->offs.soft_start += yrel) > tl->item_h) {
			tl->offs.soft_start = 0;
			if (--tl->offs.start < 0) {
				tl->offs.start = 0;
			}
		}
	}
}

static void
tlist_mouse_button(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	int sy;

	if (x > WIDGET(tl)->w - WIDGET(tl->vbar)->w) {
		event_post(tl->vbar, "window-mousebuttondown", "%p, %p, %p",
		    button, x, y);
		return;
	}

	y -= tl->offs.soft_start;

	WIDGET_FOCUS(tl);

	if (button != SELECTION_MOUSE_BUTTON) {
		return;
	}

	sy = tl->offs.start + y/tl->item_h;
	if ((tl->offs.sel = sy) > tl->nitems - 1) {
		tl->offs.sel = tl->nitems - 1;
	}

	event_post(tl, "tlist-selection-changed", "%i", sy);
}

static void
tlist_keydown(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&tl->items_lock);
	switch (keysym) {
	case SDLK_UP:
		if (--tl->offs.sel < 0) {
			tl->offs.sel = 0;
		}
		if (tl->offs.sel <= tl->offs.start) {
			if (--tl->offs.start < 0) {
				tl->offs.start = 0;
			}
		}
		event_post(tl, "tlist-selection-changed", "%i", tl->offs.sel);
		break;
	case SDLK_DOWN:
		if (++tl->offs.sel > tl->nitems) {
			tl->offs.sel = tl->nitems;
		}
		event_post(tl, "tlist-selection-changed", "%i", tl->offs.sel);
		break;
	case SDLK_LEFT:
		if (--tl->offs.start < 0) {
			tl->offs.start = 0;
		}
		break;
	case SDLK_RIGHT:
		if (++tl->offs.start > tl->nitems) {
			tl->offs.start = tl->nitems;
		}
		break;
	case SDLK_PAGEUP:
		if ((tl->offs.start -= 4) < 0) {
			tl->offs.start = 0;
		}
		break;
	case SDLK_PAGEDOWN:
		if ((tl->offs.start += 4) > tl->nitems) {
			tl->offs.start = tl->nitems;
		}
		break;
	default:
	}
	
	pthread_mutex_unlock(&tl->items_lock);
}

