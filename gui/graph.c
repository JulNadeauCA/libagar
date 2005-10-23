/*	$Csoft: graph.c,v 1.56 2005/10/01 14:15:38 vedge Exp $	*/

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

#include <core/core.h>
#include <core/view.h>

#include <gui/window.h>
#include <gui/primitive.h>

#include <string.h>

#include "graph.h"

const AG_WidgetOps agGraphOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_GraphDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GraphDraw,
	AG_GraphScale
};

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void	graph_key(AG_Event *);
static void	graph_mousemotion(AG_Event *);
static void	graph_focus(AG_Event *);
static void	graph_resume_scroll(AG_Event *);

AG_Graph *
AG_GraphNew(void *parent, enum ag_graph_type type, Uint flags)
{
	AG_Graph *graph;

	graph = Malloc(sizeof(AG_Graph), M_OBJECT);
	AG_GraphInit(graph, type, flags);
	AG_ObjectAttach(parent, graph);
	if (flags & AG_GRAPH_FOCUS) {
		AG_WidgetFocus(graph);
	}
	return (graph);
}

void
AG_GraphInit(AG_Graph *graph, enum ag_graph_type type, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_GRAPH_WFILL) { wflags |= AG_WIDGET_WFILL; }
	if (flags & AG_GRAPH_HFILL) { wflags |= AG_WIDGET_HFILL; }

	AG_WidgetInit(graph, "graph", &agGraphOps, wflags);
	graph->type = type;
	graph->flags = flags;
	graph->xoffs = 0;
	graph->origin_y = 50;
	graph->yrange = 100;
	TAILQ_INIT(&graph->items);

	AG_SetEvent(graph, "window-mousemotion", graph_mousemotion, NULL);
	AG_SetEvent(graph, "window-keydown", graph_key, NULL);
	AG_SetEvent(graph, "window-mousebuttonup", graph_resume_scroll, NULL);
	AG_SetEvent(graph, "window-mousebuttondown", graph_focus, NULL);
}

void
AG_GraphSetRange(AG_Graph *graph, AG_GraphValue range)
{
	graph->yrange = range;
}

static void
graph_key(AG_Event *event)
{
	AG_Graph *gra = AG_SELF();
	SDLKey key = AG_SDLKEY(1);

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
graph_mousemotion(AG_Event *event)
{
	AG_Graph *gra = AG_SELF();
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int state = AG_INT(5);

	if ((state & SDL_BUTTON_LMASK) == 0)
		return;

	if ((gra->xoffs -= xrel) < 0)
		gra->xoffs = 0;

	gra->origin_y += yrel;
	if (gra->origin_y < 0)
		gra->origin_y = 0;
	if (gra->origin_y > AGWIDGET(gra)->h)
		gra->origin_y = AGWIDGET(gra)->h;
}

static void
graph_resume_scroll(AG_Event *event)
{
	AG_Graph *gra = AG_SELF();

	gra->flags |= AG_GRAPH_SCROLL;
}

static void
graph_focus(AG_Event *event)
{
	AG_Graph *gra = AG_SELF();

	gra->flags &= ~(AG_GRAPH_SCROLL);
	AG_WidgetFocus(gra);
}

void
AG_GraphScale(void *p, int w, int h)
{
	AG_Graph *gra = p;

	if (w == -1 && h == -1) {
		AGWIDGET(gra)->w = 100;
		AGWIDGET(gra)->h = 80;
	}
}

void
AG_GraphDraw(void *p)
{
	AG_Graph *gra = p;
	AG_GraphItem *gi;
	int x, y, ox = 0, oy;
	Uint32 i, origin_y;
	AG_GraphValue oval;

	origin_y = AGWIDGET(gra)->h * gra->origin_y / 100;

	agPrim.box(gra,
	    0, 0,
	    AGWIDGET(gra)->w, AGWIDGET(gra)->h,
	    0,
	    AG_COLOR(GRAPH_BG_COLOR));

	if (gra->flags & AG_GRAPH_ORIGIN) {
		agPrim.hline(gra,
		    0,
		    AGWIDGET(gra)->w-1,
		    origin_y + 1,
		    AG_COLOR(GRAPH_XAXIS_COLOR));
	}

	TAILQ_FOREACH(gi, &gra->items, items) {
		if (gra->xoffs > gi->nvals || gra->xoffs < 0)
			continue;

		for (x = 2, ox = 0, i = gra->xoffs;
		     ++i < gi->nvals && x < AGWIDGET(gra)->w;
		     ox = x, x += 2) {

			oval = gi->vals[i] * AGWIDGET(gra)->h / gra->yrange;
			y = origin_y - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * AGWIDGET(gra)->h /
				    gra->yrange;
				oy = origin_y - oval;
			} else {
				oy = origin_y;
			}

			if (y < 0)
				y = 0;
			if (oy < 0)
				oy = 0;
			if (y > AGWIDGET(gra)->h)
				y = AGWIDGET(gra)->h;
			if (oy > AGWIDGET(gra)->h)
				oy = AGWIDGET(gra)->h;

			switch (gra->type) {
			case AG_GRAPH_POINTS:
				AG_WidgetPutPixel(gra, x, y, gi->color);
				break;
			case AG_GRAPH_LINES:
				{
					agPrim.line(gra,
					    ox, oy,
					    x, y,
					    gi->color);
				}
				break;
			}
		}
	}
}

AG_GraphItem *
AG_GraphAddItem(AG_Graph *gra, const char *name, Uint8 r, Uint8 g, Uint8 b,
    Uint32 limit)
{
	AG_GraphItem *gi;

 	gi = Malloc(sizeof(AG_GraphItem), M_WIDGET);
	strlcpy(gi->name, name, sizeof(gi->name));
	gi->color = SDL_MapRGB(agVideoFmt, r, g, b);
	gi->vals = Malloc(NITEMS_INIT * sizeof(AG_GraphValue), M_WIDGET);
	gi->maxvals = NITEMS_INIT;
	gi->nvals = 0;
	gi->graph = gra;
	gi->limit = limit>0 ? limit : (0xffffffff-1);
	TAILQ_INSERT_HEAD(&gra->items, gi, items);
	return (gi);
}

void
AG_GraphPlot(AG_GraphItem *gi, AG_GraphValue val)
{
	gi->vals[gi->nvals] = val;

	if (gi->nvals+1 >= gi->limit) {
		memmove(gi->vals, gi->vals+1, gi->nvals*sizeof(AG_GraphValue));
		gi->graph->flags &= ~(AG_GRAPH_SCROLL);
	} else {
		if (gi->nvals+1 >= gi->maxvals) {
			gi->vals = Realloc(gi->vals,
			    (gi->maxvals+NITEMS_GROW) * sizeof(AG_GraphValue));
			gi->maxvals += NITEMS_GROW;
		}
		gi->nvals++;
	}
}

void
AG_GraphScroll(AG_Graph *gra, int i)
{
	if (gra->flags & AG_GRAPH_SCROLL)
		gra->xoffs += i;
}

void
AG_GraphFreeItems(AG_Graph *gra)
{
	AG_GraphItem *git, *nextgit;
	
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
AG_GraphDestroy(void *p)
{
	AG_Graph *gra = p;

	AG_GraphFreeItems(gra);
	AG_WidgetDestroy(gra);
}
