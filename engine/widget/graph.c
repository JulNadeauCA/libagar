/*	$Csoft: graph.c,v 1.1 2002/07/20 18:55:59 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/version.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "graph.h"
#include "primitive.h"

static const struct widget_ops graph_ops = {
	{
		graph_destroy,
		NULL,	/* load */
		NULL	/* save */
	},
	graph_draw,
	NULL		/* animate */
};

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void	graph_scroll(int, union evarg *);
static void	graph_focus(int, union evarg *);
static void	graph_resume_scroll(int, union evarg *);

struct graph *
graph_new(struct region *reg, const char *caption, enum graph_type t, int flags,
    int w, int h)
{
	struct graph *graph;

	graph = emalloc(sizeof(struct graph));
	graph_init(graph, caption, t, flags, w, h);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, graph);
	pthread_mutex_unlock(&reg->win->lock);

	return (graph);
}

void
graph_init(struct graph *graph, const char *caption, enum graph_type t,
    int flags, int w, int h)
{
	widget_init(&graph->wid, "graph", "widget", &graph_ops, w, h);
	graph->type = t;
	graph->flags = flags;
	graph->caption = strdup(caption);
	graph->xoffs = 0;
	graph->xinc = 2;
	TAILQ_INIT(&graph->items);

	event_new(graph, "window-mousemotion", 0, graph_scroll, NULL);
	event_new(graph, "window-mousebuttonup", 0, graph_resume_scroll, NULL);
	event_new(graph, "window-mousebuttondown", 0, graph_focus, NULL);
}

static void
graph_scroll(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;
	int xrel = argv[3].i, yrel = argv[4].i;

	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
		gra->xoffs -= xrel;
		if (gra->xoffs < 0) {
			gra->xoffs = 0;
		}
	}
}

static void
graph_resume_scroll(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;

	gra->flags |= GRAPH_SCROLL;
}

static void
graph_focus(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;

	gra->flags &= ~(GRAPH_SCROLL);
	WIDGET_FOCUS(gra);
}

void
graph_draw(void *p)
{
	struct graph *gra = p;
	struct graph_item *gi;
	int x, y, oy, orig_y, i;
	Sint32 *val;

	orig_y = WIDGET(gra)->h / 2;

	TAILQ_FOREACH(gi, &gra->items, items) {
		if (gra->xoffs > gi->nvals) {
			continue;
		}
		for (x = 1, i = gra->xoffs; ++i < gi->nvals; x += gra->xinc) {
			if (x > WIDGET(gra)->w) {
				if (gra->flags & GRAPH_SCROLL) {
					gra->xoffs++;
					x--;
				} else {
					continue;
				}
			}
			y = orig_y - gi->vals[i];
			if (y > 0 && y < WIDGET(gra)->h) {
				switch (gra->type) {
				case GRAPH_POINTS:
					WIDGET_PUT_PIXEL(gra, x, y, gi->color);
					break;
				case GRAPH_LINES:
					if (i > 1) {
						oy = orig_y - gi->vals[i-1];
					} else {
						oy = orig_y;
					}
					primitives.line(gra,
					    x - gra->xinc, oy, x, y, gi->color);
					break;
				}
			}
		}
	}
}

struct graph_item *
graph_add_item(struct graph *gra, char *name, Uint32 color)
{
	struct graph_item *gi;

 	gi = emalloc(sizeof(struct graph_item));
	gi->name = strdup(name);
	gi->color = color;
	gi->vals = NULL;
	gi->nvals = 0;

	TAILQ_INSERT_HEAD(&gra->items, gi, items);

	return (gi);
}

void
graph_plot(struct graph_item *gi, Sint32 val)
{
	if (gi->vals == NULL) {				/* Initialize */
		gi->vals = emalloc(NITEMS_INIT * sizeof(Sint32));
		gi->maxvals = NITEMS_INIT;
		gi->nvals = 0;
	} else if (gi->nvals >= gi->maxvals) {		/* Grow */
		Sint32 *newvals;

		newvals = erealloc(gi->vals,
		    (NITEMS_GROW * gi->maxvals) * sizeof(Sint32));
		gi->maxvals += NITEMS_GROW;
		gi->vals = newvals;
	}
	gi->vals[gi->nvals++] = val;
}

void
graph_destroy(void *p)
{
	struct graph *gra = p;
	struct graph_item *git, *nextgit;

	free(gra->caption);

	for (git = TAILQ_FIRST(&gra->items);
	     git != TAILQ_LAST(&gra->items, itemq);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		free(git);
	}
}
