/*	$Csoft: graph.c,v 1.33 2003/06/08 00:21:04 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/view.h>

#include "graph.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

const struct version graph_ver = {
	"agar graph",
	2, 0
};

const struct widget_ops graph_ops = {
	{
		NULL,		/* init */
		graph_destroy,
		graph_load,
		graph_save,
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
static void	graph_move(int, union evarg *);
static void	graph_focus(int, union evarg *);
static void	graph_resume_scroll(int, union evarg *);

struct graph *
graph_new(void *parent, const char *caption, enum graph_type type, int flags,
    Sint32 yrange)
{
	struct graph *graph;

	graph = Malloc(sizeof(struct graph));
	graph_init(graph, caption, type, flags, yrange);
	object_attach(parent, graph);
	return (graph);
}

void
graph_init(struct graph *graph, const char *caption, enum graph_type type,
    int flags, Sint32 yrange)
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
	graph->xinc = 2;
	graph->yrange = yrange;
	graph->origin_y = 50;
	graph->yrange = yrange;
	TAILQ_INIT(&graph->items);

	event_new(graph, "window-mousemotion", graph_move, NULL);
	event_new(graph, "window-keydown", graph_key, NULL);
	event_new(graph, "window-mousebuttonup", graph_resume_scroll, NULL);
	event_new(graph, "window-mousebuttondown", graph_focus, NULL);
}

int
graph_load(void *p, struct netbuf *buf)
{
	struct graph *gra = p;
	Uint32 nitems, i;

	if (version_read(buf, &graph_ver, NULL) == -1)
		return (-1);

	gra->type = read_uint32(buf);
	gra->flags = (int)read_uint32(buf);
	gra->origin_y = (int)read_uint8(buf);
	gra->xinc = (int)read_uint8(buf);
	gra->yrange = read_sint32(buf);
	gra->xoffs = read_sint32(buf);
	copy_string(gra->caption, buf, sizeof(gra->caption));

	graph_free_items(gra);
	nitems = read_uint32(buf);
	for (i = 0; i < nitems; i++) {
		char name[GRAPH_ITEM_NAME_MAX];
		struct graph_item *nitem;
		Uint32 vi, nvals;
		Uint8 r, g, b;

		copy_string(name, buf, sizeof(name));
		r = read_uint8(buf);
		g = read_uint8(buf);
		b = read_uint8(buf);
		nvals = read_uint32(buf);
		nitem = graph_add_item(gra, name, r, g, b);

		for (vi = 0; vi < nvals; vi++)
			graph_plot(nitem, read_sint32(buf));
	}

	dprintf("loaded `%s'\n", gra->caption);
	return (0);
}

int
graph_save(void *p, struct netbuf *buf)
{
	struct graph *gra = p;
	struct graph_item *gi;
	Uint32 nitems = 0;
	off_t nitems_offs;

	version_write(buf, &graph_ver);
	write_uint32(buf, gra->type);
	write_uint32(buf, (Uint32)gra->flags);
	write_uint8(buf, (Uint8)gra->origin_y);
	write_uint8(buf, (Uint8)gra->xinc);
	write_uint32(buf, gra->yrange);
	write_sint32(buf, gra->xoffs);
	write_string(buf, gra->caption);
	
	TAILQ_FOREACH(gi, &gra->items, items)
		nitems++;

	nitems_offs = buf->offs;
	write_uint32(buf, 0);
	TAILQ_FOREACH(gi, &gra->items, items) {
		Uint32 i;

		write_string(buf, gi->name);
		write_uint32(buf, gi->color);
		write_uint32(buf, gi->nvals);
		for (i = 0; i < gi->nvals; i++)
			write_sint32(buf, gi->vals[i]);
	}
	pwrite_uint32(buf, nitems, nitems_offs);

	dprintf("saved `%s'\n", gra->caption);
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
		if ((gra->xoffs -= 10) < 0)
			gra->xoffs = 0;
		break;
	case SDLK_RIGHT:
		gra->xoffs += 10;
		break;
	case SDLK_s:
		if (object_save(gra) == -1)
			text_msg("Error saving", "%s", error_get());
		break;
	case SDLK_l:
		if (object_load(gra) == -1)
			text_msg("Error loading", "%s", error_get());
	default:
		break;
	}
}

static void
graph_move(int argc, union evarg *argv)
{
	struct graph *gra = argv[0].p;
	int xrel = argv[3].i;
	int yrel = argv[4].i;

	if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) == 0)
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
	int x, y, oy;
	Uint32 i, origin_y;
	Sint32 oval;

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
				widget_put_pixel(gra, x, y, gi->color);
				break;
			case GRAPH_LINES:
				{
					int ncolor;

					ncolor = widget_push_color(WIDGET(gra),
					    gi->color);
					primitives.line(gra,
					    x - gra->xinc,
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
graph_add_item(struct graph *gra, const char *name, Uint8 r, Uint8 g, Uint8 b)
{
	struct graph_item *gi;

 	gi = Malloc(sizeof(struct graph_item));
	strlcpy(gi->name, name, sizeof(gi->name));
	gi->color = SDL_MapRGB(vfmt, r, g, b);
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
		gi->vals = Malloc(NITEMS_INIT * sizeof(Sint32));
		gi->maxvals = NITEMS_INIT;
		gi->nvals = 0;
	} else if (gi->nvals >= gi->maxvals) {		/* Grow */
		Sint32 *newvals;

		newvals = Realloc(gi->vals,
		    (NITEMS_GROW * gi->maxvals) * sizeof(Sint32));
		gi->maxvals += NITEMS_GROW;
		gi->vals = newvals;
	}
	gi->vals[gi->nvals++] = val;
}

__inline__ void
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
	     git != TAILQ_LAST(&gra->items, itemq);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		free(git);
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
