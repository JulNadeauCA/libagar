/*	$Csoft$	*/

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

#include "menu.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <stdarg.h>
#include <string.h>

static struct widget_ops ag_menu_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		ag_menu_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	ag_menu_draw,
	ag_menu_scale
};

enum {
	UNZEL_COLOR,
	SEL_COLOR,
	TEXT_COLOR
};

struct AGMenu *
ag_menu_new(void *parent)
{
	struct AGMenu *m;

	m = Malloc(sizeof(struct AGMenu), M_OBJECT);
	ag_menu_init(m);
	object_attach(parent, m);
	return (m);
}

void
ag_menu_init(struct AGMenu *m)
{
	widget_init(m, "menu", &ag_menu_ops, WIDGET_WFILL|
	                                  WIDGET_UNFOCUSED_BUTTONUP);
	widget_map_color(m, UNZEL_COLOR, "unzelected", 100, 100, 100, 255);
	widget_map_color(m, SEL_COLOR, "selected", 50, 50, 120, 255);
	widget_map_color(m, TEXT_COLOR, "text", 240, 240, 240, 255);

	m->items = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	m->nitems = 0;
	m->vspace = 5;
	m->hspace = 17;
#if 0
	event_new(com, "window-mousebuttondown", ag_menu_mousebuttondown, NULL);
	event_new(com, "window-mousebuttonup", ag_menu_mousebuttonup, NULL);
#endif
}

struct AGMenuItem *
ag_menu_add_item(struct AGMenu *m, const char *text)
{
	struct AGMenuItem *mit;
	
	m->items = Realloc(m->items, (m->nitems+1)*sizeof(struct AGMenuItem));
	mit = &m->items[m->nitems++];
	mit->text = text;
	mit->surface = text_render(NULL, -1, TEXT_COLOR, text);
	mit->key_equiv = 0;
	mit->key_mod = 0;
	mit->fn = NULL;
	mit->subitems = NULL;
	mit->nsubitems = 0;
	return (mit);
}

struct AGMenuItem *
ag_menu_add_subitem(struct AGMenuItem *pmit, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLMod key_mod, void (*fn)(void *), void *arg)
{
	struct AGMenuItem *mit;
	
	if (pmit->subitems == NULL) {
		pmit->subitems = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	} else {
		pmit->subitems = Realloc(pmit->subitems,
		    (pmit->nsubitems+1)*sizeof(struct AGMenuItem));
	}
	mit = &pmit->subitems[pmit->nsubitems++];
	mit->text = text;
	mit->surface = text_render(NULL, -1, TEXT_COLOR, text);
	/* TODO icon */
	mit->key_equiv = key_equiv;
	mit->key_mod = key_mod;
	mit->fn = fn;
	mit->arg = arg;
	mit->subitems = NULL;
	mit->nsubitems = 0;
	return (mit);
}

static void
free_subitems(struct AGMenuItem *mit)
{
	int i;

	if (mit->surface != NULL) {
		SDL_FreeSurface(mit->surface);
	}
	for (i = 0; i < mit->nsubitems; i++) {
		free_subitems(&mit->subitems[i]);
	}
	Free(mit->subitems, M_WIDGET);
}

void
ag_menu_free_items(struct AGMenu *m)
{
	int i;

	for (i = 0; i < m->nitems; i++) {
		free_subitems(&m->items[i]);
	}
	m->nitems = 0;
}

void
ag_menu_destroy(void *p)
{
	struct AGMenu *m = p;

	ag_menu_free_items(m);
	Free(m->items, M_WIDGET);
	widget_destroy(m);
}

void
ag_menu_draw(void *p)
{
	struct AGMenu *m = p;
	int x = m->hspace;
	int y = m->vspace;
	int i;

	if (WIDGET(m)->w < m->hspace*2 ||
	    WIDGET(m)->h < m->vspace*2)
		return;

	primitives.box(m, 0, 0, WIDGET(m)->w, WIDGET(m)->h, 1, UNZEL_COLOR);
	
	for (i = 0; i < m->nitems; i++) {
		struct AGMenuItem *mit = &m->items[i];

		if (mit->surface == NULL) {
			continue;
		}
		widget_blit(m, mit->surface, x, y);
		x += mit->surface->w + m->hspace;
		if (x > view->w/2) {
			x = m->hspace;
			y += mit->surface->h + m->vspace;
		}
	}
}

void
ag_menu_scale(void *p, int w, int h)
{
	struct AGMenu *m = p;

	if (w == -1 && h == -1) {
		int x, y;
		int i;

		x = WIDGET(m)->w = m->hspace;
		y = WIDGET(m)->h = m->vspace;

		for (i = 0; i < m->nitems; i++) {
			struct AGMenuItem *mit = &m->items[i];

			if (mit->surface == NULL) {
				continue;
			}
			x += mit->surface->w+m->hspace;
			if (WIDGET(m)->h < mit->surface->h) {
				WIDGET(m)->h += mit->surface->h + m->vspace;
			}
			if (x > view->w/2) {
				x = m->hspace;			/* Wrap */
				y += mit->surface->h + m->vspace;
				WIDGET(m)->h += mit->surface->h;
			} else {
				WIDGET(m)->w += mit->surface->w + m->hspace;
			}
		}
	}
}

