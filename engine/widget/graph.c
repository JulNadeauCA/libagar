/*	$Csoft: graph.c,v 1.13 2002/09/08 03:45:01 vedge Exp $	*/

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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/version.h>

#include <libfobj/fobj.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "graph.h"
#include "primitive.h"

static const struct version graph_ver = {
	"agar graph",
	2, 0
};

static const struct widget_ops graph_ops = {
	{
		graph_destroy,
		graph_load,
		graph_save
	},
	graph_draw,
	NULL		/* update */
};

enum {
	FRAME_COLOR,
	TEXT_COLOR,
	ORIGIN_COLOR1,
	ORIGIN_COLOR2
};

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void	graph_key(int, union evarg *);
static void	graph_scroll(int, union evarg *);
static void	graph_focus(int, union evarg *);
static void	graph_resume_scroll(int, union evarg *);

struct graph *
graph_new(struct region *reg, const char *caption, enum graph_type t,
    Uint32 flags, Sint32 yrange, int w, int h)
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
    Uint32 flags, Sint32 yrange, int w, int h)
{
	widget_init(&graph->wid, "graph", "widget", &graph_ops, w, h);
	widget_map_color(graph, FRAME_COLOR, "graph-frame", 50, 50, 50);
	widget_map_color(graph, TEXT_COLOR, "graph-text", 200, 200, 200);
	widget_map_color(graph, ORIGIN_COLOR1, "graph-origin1", 50, 50, 50);
	widget_map_color(graph, ORIGIN_COLOR2, "graph-origin2", 150, 150, 150);
	
	graph->type = t;
	graph->flags = flags;
	graph->caption = strdup(caption);
	graph->xoffs = 0;
	graph->xinc = 2;
	graph->yrange = yrange;
	graph->origin_y = 50;
	graph->yrange = yrange;
	TAILQ_INIT(&graph->items);

	event_new(graph, "window-mousemotion", 0, graph_scroll, NULL);
	event_new(graph, "window-keydown", 0, graph_key, NULL);
	event_new(graph, "window-mousebuttonup", 0, graph_resume_scroll, NULL);
	event_new(graph, "window-mousebuttondown", 0, graph_focus, NULL);
}

int
graph_load(void *p, int fd)
{
	struct graph *gra = p;
	Uint32 nitems, i;

	version_read(fd, &graph_ver);
	gra->type = read_uint32(fd);
	gra->flags = read_uint32(fd);
	gra->origin_y = read_uint8(fd);
	gra->xinc = read_uint8(fd);
	gra->yrange = read_sint32(fd);
	gra->xoffs = read_uint32(fd);
	gra->caption = read_string(fd);
	
	nitems = read_uint32(fd);
	for (i = 0; i < nitems; i++) {
		struct graph_item *nitem;
		Uint32 vi, color, nvals;
		char *s;

		s = read_string(fd);
		color = read_uint32(fd);
		nvals = read_uint32(fd);

		nitem = graph_add_item(gra, s, color);

		for (vi = 0; vi < nvals; vi++) {
			graph_plot(nitem, read_sint32(fd));
		}
	}

	dprintf("loaded %s\n", gra->caption);
	return (0);
}

int
graph_save(void *p, int fd)
{
	struct graph *gra = p;
	struct graph_item *gi;
	Uint32 nitems = 0;

	version_write(fd, &graph_ver);
	write_uint32(fd, gra->type);
	write_uint32(fd, gra->flags);
	write_uint8(fd, gra->origin_y);
	write_uint8(fd, gra->xinc);
	write_uint32(fd, gra->yrange);
	write_uint32(fd, gra->xoffs);
	write_string(fd, gra->caption);
	
	TAILQ_FOREACH(gi, &gra->items, items) {
		nitems++;
	}
	write_uint32(fd, nitems);

	TAILQ_FOREACH(gi, &gra->items, items) {
		Uint32 i;

		write_string(fd, gi->name);
		write_uint32(fd, gi->color);
		write_uint32(fd, gi->nvals);
		for (i = 0; i < gi->nvals; i++) {
			write_sint32(fd, gi->vals[i]);
		}
	}

	dprintf("saved %s\n", gra->caption);
	return (0);
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
		if ((gra->xoffs -= 10) < 0) {
			gra->xoffs = 0;
		}
		break;
	case SDLK_RIGHT:
		gra->xoffs += 10;
		break;
	case SDLK_s:
		pthread_mutex_lock(&world->lock);
		object_save(gra);
		pthread_mutex_unlock(&world->lock);
		break;
	case SDLK_l:
		pthread_mutex_lock(&world->lock);
		object_load(gra);
		pthread_mutex_unlock(&world->lock);
	default:
	}
}

static void
graph_scroll(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;
	int xrel = argv[3].i;
	int yrel = argv[4].i;

	if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) == 0) {
		return;
	}

	gra->xoffs -= xrel;
	if (gra->xoffs < 0)
		gra->xoffs = 0;

	gra->origin_y += yrel;
	if (gra->origin_y > 100)
		gra->origin_y = 100;
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
	int x, y, oy;
	Uint32 i, origin_y;
	Sint32 oval;

	origin_y = WIDGET(gra)->h * gra->origin_y / 100;

	primitives.box(gra, 0, 0, WIDGET(gra)->w, WIDGET(gra)->h, 0,
	    WIDGET_COLOR(gra, FRAME_COLOR));

	if (gra->flags & GRAPH_ORIGIN) {
		primitives.line(gra, 0, origin_y, WIDGET(gra)->w, origin_y,
		    WIDGET_FOCUSED(gra) ?
		    WIDGET_COLOR(gra, ORIGIN_COLOR1) :
		    WIDGET_COLOR(gra, ORIGIN_COLOR2));
		primitives.line(gra, 0, origin_y+1,
		    WIDGET(gra)->w, origin_y+1,
		    WIDGET_FOCUSED(gra) ?
		    WIDGET_COLOR(gra, ORIGIN_COLOR2) :
		    WIDGET_COLOR(gra, ORIGIN_COLOR1));
	}

	TAILQ_FOREACH(gi, &gra->items, items) {
		if (gra->xoffs > gi->nvals || gra->xoffs < 0) {
			continue;
		}
		for (x = 1, i = gra->xoffs;
		     ++i < gi->nvals && x < WIDGET(gra)->w;
		     x += gra->xinc) {

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
	gi->graph = gra;

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
	
	if (gi->graph->flags & GRAPH_SCROLL &&
	    gi->nvals > WIDGET(gi->graph)->w/2) {
		gi->graph->xoffs++;
	}
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

	widget_destroy(gra);
}
