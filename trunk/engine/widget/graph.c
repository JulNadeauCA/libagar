/*	$Csoft: graph.c,v 1.2 2002/07/21 10:57:12 vedge Exp $	*/

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
static void	graph_scaled(int, union evarg *);

struct graph *
graph_new(struct region *reg, const char *caption, enum graph_type t, int flags,
    Sint32 yrange, int w, int h)
{
	struct graph *graph;

	graph = emalloc(sizeof(struct graph));
	graph_init(graph, caption, t, flags, yrange, w, h);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, graph);
	pthread_mutex_unlock(&reg->win->lock);

	return (graph);
}

void
graph_init(struct graph *graph, const char *caption, enum graph_type t,
    int flags, Sint32 yrange, int w, int h)
{
	widget_init(&graph->wid, "graph", "widget", &graph_ops, w, h);
	graph->type = t;
	graph->flags = flags;
	graph->caption = strdup(caption);
	graph->xoffs = 0;
	graph->xinc = 2;
	graph->yrange = yrange;
	graph->origin_color[0] = SDL_MapRGB(view->v->format, 150, 150, 150);
	graph->origin_color[1] = SDL_MapRGB(view->v->format, 60, 60, 60);
	graph->yrange = yrange;
	TAILQ_INIT(&graph->items);

	event_new(graph, "window-mousemotion", 0, graph_scroll, NULL);
	event_new(graph, "window-mousebuttonup", 0, graph_resume_scroll, NULL);
	event_new(graph, "window-mousebuttondown", 0, graph_focus, NULL);
	event_new(graph, "widget-scaled", 0, graph_scaled, NULL);
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
		gra->origin_y += yrel;
		if (gra->origin_y < 0)
			gra->origin_y = 0;
		if (gra->origin_y > WIDGET(gra)->h)
			gra->origin_y = WIDGET(gra)->h;
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

static void
graph_scaled(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;

	gra->origin_y = WIDGET(gra)->h / 2;
}

void
graph_draw(void *p)
{
	struct graph *gra = p;
	struct graph_item *gi;
	int x, y, oy, i;
	Sint32 *val, oval;

	primitives.box(gra, 0, 0, WIDGET(gra)->w, WIDGET(gra)->h, 0);

	if (gra->flags & GRAPH_ORIGIN) {
		primitives.line(gra, 0, gra->origin_y,
		    WIDGET(gra)->w, gra->origin_y,
		    WIDGET_FOCUSED(gra) ?
		    gra->origin_color[0] : gra->origin_color[1]);
		primitives.line(gra, 0, gra->origin_y+1,
		    WIDGET(gra)->w, gra->origin_y+1,
		    WIDGET_FOCUSED(gra) ?
		    gra->origin_color[1] : gra->origin_color[0]);
	}

	TAILQ_FOREACH(gi, &gra->items, items) {
		if (gra->xoffs > gi->nvals) {
			continue;
		}
		for (x = 1, i = gra->xoffs; ++i < gi->nvals; x += gra->xinc) {
			if (x > WIDGET(gra)->w) {
				if (gra->flags & GRAPH_SCROLL) {
					gra->xoffs += gra->xinc;
					x -= gra->xinc;
				} else {
					continue;
				}
			}

			oval = gi->vals[i] * WIDGET(gra)->h / gra->yrange;
			y = gra->origin_y - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * WIDGET(gra)->h /
				    gra->yrange;
				oy = gra->origin_y - oval;
			} else {
				oy = gra->origin_y;
			}

			if (y < 0)
				y = 0;
			if (oy < 0)
				oy = 0;
			if (y > WIDGET(gra)->h)
				y = WIDGET(gra)->h;
			if (oy > WIDGET(gra)->h)
				oy = WIDGET(gra)->h;

			switch (gra->type) {
			case GRAPH_POINTS:
				WIDGET_PUT_PIXEL(gra, x, y, gi->color);
				break;
			case GRAPH_LINES:
				primitives.line(gra,
				    x - gra->xinc, oy,
				    x, y, gi->color);
				break;
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
