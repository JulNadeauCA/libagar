/*	$Csoft: graph.c,v 1.49 2004/06/18 03:11:30 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <string.h>

#include "graph.h"

const struct version graph_ver = {
	"agar graph",
	2, 0
};

const struct widget_ops graph_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		graph_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	graph_draw,
	graph_scale
};

enum {
	FRAME_COLOR,
	ORIGIN_COLOR1,
	ORIGIN_COLOR2
};

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void	graph_key(int, union evarg *);
static void	graph_mousemotion(int, union evarg *);
static void	graph_focus(int, union evarg *);
static void	graph_resume_scroll(int, union evarg *);

struct graph *
graph_new(void *parent, const char *caption, enum graph_type type, int flags,
    graph_val_t yrange)
{
	struct graph *graph;

	graph = Malloc(sizeof(struct graph), M_OBJECT);
	graph_init(graph, caption, type, flags, yrange);
	object_attach(parent, graph);
	return (graph);
}

void
graph_init(struct graph *graph, const char *caption, enum graph_type type,
    int flags, graph_val_t yrange)
{
	widget_init(graph, "graph", &graph_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL|WIDGET_HFILL);

	widget_map_color(graph, FRAME_COLOR, "frame", 50, 50, 50, 255);
	widget_map_color(graph, ORIGIN_COLOR1, "origin1", 50, 50, 50, 255);
	widget_map_color(graph, ORIGIN_COLOR2, "origin2", 150, 150, 150, 255);
	
	strlcpy(graph->caption, caption, sizeof(graph->caption));
	graph->type = type;
	graph->flags = flags;
	graph->xoffs = 0;
	graph->yrange = yrange;
	graph->origin_y = 50;
	graph->yrange = yrange;
	TAILQ_INIT(&graph->items);

	event_new(graph, "window-mousemotion", graph_mousemotion, NULL);
	event_new(graph, "window-keydown", graph_key, NULL);
	event_new(graph, "window-mousebuttonup", graph_resume_scroll, NULL);
	event_new(graph, "window-mousebuttondown", graph_focus, NULL);
}

static void
graph_key(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;
	SDLKey key = (SDLKey)argv[1].p;

	switch (key) {
	case SDLK_0:
		gra->xoffs = 0;
		break;
	case SDLK_LEFT:
		if ((gra->xoffs -= 10) < 0)
			gra->xoffs = 0;
		break;
	case SDLK_RIGHT:
		gra->xoffs += 10;
		break;
	default:
		break;
	}
}

static void
graph_mousemotion(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	int state = argv[5].i;

	if ((state & SDL_BUTTON_LMASK) == 0)
		return;

	if ((gra->xoffs -= xrel) < 0)
		gra->xoffs = 0;

	gra->origin_y += yrel;
	if (gra->origin_y < 0)
		gra->origin_y = 0;
	if (gra->origin_y > WIDGET(gra)->h)
		gra->origin_y = WIDGET(gra)->h;
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
	widget_focus(gra);
}

void
graph_scale(void *p, int w, int h)
{
	struct graph *gra = p;

	if (w == -1 && h == -1) {
		/* XXX more sensible default */
		WIDGET(gra)->w = 150;
		WIDGET(gra)->h = 100;
	}
}

void
graph_draw(void *p)
{
	struct graph *gra = p;
	struct graph_item *gi;
	int x, y, ox = 0, oy;
	Uint32 i, origin_y;
	graph_val_t oval;

	origin_y = WIDGET(gra)->h * gra->origin_y / 100;

	primitives.box(gra,
	    0, 0,
	    WIDGET(gra)->w, WIDGET(gra)->h,
	    0,
	    FRAME_COLOR);

	if (gra->flags & GRAPH_ORIGIN) {
		primitives.line(gra,
		    0,
		    origin_y,
		    WIDGET(gra)->w,
		    origin_y,
		    widget_holds_focus(gra) ? ORIGIN_COLOR1 : ORIGIN_COLOR2);
		primitives.line(gra,
		    0,
		    origin_y + 1,
		    WIDGET(gra)->w,
		    origin_y + 1,
		    widget_holds_focus(gra) ? ORIGIN_COLOR2 : ORIGIN_COLOR1);
	}

	TAILQ_FOREACH(gi, &gra->items, items) {
		if (gra->xoffs > gi->nvals || gra->xoffs < 0)
			continue;

		for (x = 2, ox = 0, i = gra->xoffs;
		     ++i < gi->nvals && x < WIDGET(gra)->w;
		     ox = x, x += 2) {

			oval = gi->vals[i] * WIDGET(gra)->h / gra->yrange;
			y = origin_y - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * WIDGET(gra)->h /
				    gra->yrange;
				oy = origin_y - oval;
			} else {
				oy = origin_y;
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
				widget_put_pixel(gra, x, y, gi->color);
				break;
			case GRAPH_LINES:
				{
					int ncolor;

					ncolor = widget_push_color(WIDGET(gra),
					    gi->color);
					primitives.line(gra,
					    ox,
					    oy,
					    x,
					    y,
					    ncolor);
					widget_pop_color(WIDGET(gra));
				}
				break;
			}
		}
	}
}

struct graph_item *
graph_add_item(struct graph *gra, const char *name, Uint8 r, Uint8 g, Uint8 b,
    Uint32 limit)
{
	struct graph_item *gi;

 	gi = Malloc(sizeof(struct graph_item), M_WIDGET);
	strlcpy(gi->name, name, sizeof(gi->name));
	gi->color = SDL_MapRGB(vfmt, r, g, b);
	gi->vals = Malloc(NITEMS_INIT * sizeof(graph_val_t), M_WIDGET);
	gi->maxvals = NITEMS_INIT;
	gi->nvals = 0;
	gi->graph = gra;
	gi->limit = limit>0 ? limit : (0xffffffff-1);
	TAILQ_INSERT_HEAD(&gra->items, gi, items);
	return (gi);
}

void
graph_plot(struct graph_item *gi, graph_val_t val)
{
	gi->vals[gi->nvals] = val;

	if (gi->nvals+1 >= gi->limit) {
		memmove(gi->vals, gi->vals+1, gi->nvals*sizeof(graph_val_t));
		gi->graph->flags &= ~(GRAPH_SCROLL);
	} else {
		if (gi->nvals+1 >= gi->maxvals) {
			gi->vals = Realloc(gi->vals,
			    (gi->maxvals+NITEMS_GROW) * sizeof(graph_val_t));
			gi->maxvals += NITEMS_GROW;
		}
		gi->nvals++;
	}
}

void
graph_scroll(struct graph *gra, int i)
{
	if (gra->flags & GRAPH_SCROLL)
		gra->xoffs += i;
}

void
graph_free_items(struct graph *gra)
{
	struct graph_item *git, *nextgit;
	
	for (git = TAILQ_FIRST(&gra->items);
	     git != TAILQ_END(&gra->items);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		Free(git->vals, M_WIDGET);
		Free(git, M_WIDGET);
	}
	TAILQ_INIT(&gra->items);
}

void
graph_destroy(void *p)
{
	struct graph *gra = p;

	graph_free_items(gra);
	widget_destroy(gra);
}
