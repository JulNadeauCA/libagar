/*	$Csoft: mgraph.c,v 1.3 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef FLOATING_POINT

#include "mgraph.h"

#include <engine/view.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <stdarg.h>

const struct version mgraph_ver = {
	"agar mgraph",
	0, 0
};

const struct widget_ops mgraph_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		mgraph_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	mgraph_draw,
	mgraph_scale
};

enum {
	FRAME_COLOR,
	ORIGIN_COLOR1,
	ORIGIN_COLOR2
};

static void	mgraph_focus(int, union evarg *);

struct mgraph *
mgraph_new(void *parent, enum mgraph_style style, const char *name_fmt, ...)
{
	char name[LABEL_MAX];
	struct mgraph *gra;
	va_list args;

	va_start(args, name_fmt);
	vsnprintf(name, sizeof(name), name_fmt, args);
	va_end(args);

	gra = Malloc(sizeof(struct mgraph), M_OBJECT);
	mgraph_init(gra, style, name);
	object_attach(parent, gra);
	return (gra);
}

void
mgraph_init(struct mgraph *gra, enum mgraph_style style, const char *name)
{
	widget_init(gra, "mgraph", &mgraph_ops, WIDGET_FOCUSABLE|
	    WIDGET_WFILL|WIDGET_HFILL);

	widget_map_color(gra, FRAME_COLOR, "frame", 50, 50, 50, 255);
	widget_map_color(gra, ORIGIN_COLOR1, "origin1", 50, 50, 50, 255);
	widget_map_color(gra, ORIGIN_COLOR2, "origin2", 150, 150, 150, 255);

	strlcpy(gra->name, name, sizeof(gra->name));
	gra->style = style;
	TAILQ_INIT(&gra->functions);

	event_new(gra, "window-mousebuttondown", mgraph_focus, NULL);
}

static void
mgraph_focus(int argc, union evarg *argv)
{
	struct mgraph *gra = argv[0].p;

	widget_focus(gra);
}

void
mgraph_scale(void *p, int w, int h)
{
	struct mgraph *gra = p;

	if (w == -1 && h == -1) {
		/* XXX more sensible default */
		WIDGET(gra)->w = 150;
		WIDGET(gra)->h = 100;
	}
}

void
mgraph_draw(void *p)
{
	struct mgraph *gra = p;
	struct mgraph_item *gi;
	int x, y, oy;
	Uint32 i, origin_y;
	double oval;

	primitives.box(gra,
	    0, 0,
	    WIDGET(gra)->w, WIDGET(gra)->h,
	    0,
	    FRAME_COLOR);

	primitives.line(gra,
	    0,
	    WIDGET(gra)->h/2,
	    WIDGET(gra)->w,
	    WIDGET(gra)->h/2,
	    widget_holds_focus(gra) ? ORIGIN_COLOR1 : ORIGIN_COLOR2);
	primitives.line(gra,
	    0,
	    WIDGET(gra)->h/2 + 1,
	    WIDGET(gra)->w,
	    WIDGET(gra)->h/2 + 1,
	    widget_holds_focus(gra) ? ORIGIN_COLOR2 : ORIGIN_COLOR1);
}

static void
mgraph_free_function(struct mgraph_function *fu)
{
	Free(fu->points, M_WIDGET);
	Free(fu, M_WIDGET);
}

/* Register a new function. */
struct mgraph_function *
mgraph_add_function(struct mgraph *gra, double (*func)(double),
    Uint32 npoints, Uint8 r, Uint8 g, Uint8 b, const char *fmt, ...)
{
	struct mgraph_function *fu;
	va_list vargs;

 	fu = Malloc(sizeof(struct mgraph_function), M_WIDGET);
	fu->color = SDL_MapRGB(vfmt, r, g, b);
	fu->points = NULL;
	fu->npoints = 0;
	TAILQ_INSERT_HEAD(&gra->functions, fu, functions);
	
	va_start(vargs, fmt);
	vsnprintf(fu->name, sizeof(fu->name), fmt, vargs);
	va_end(vargs);
	return (fu);
}

void
mgraph_function_set_color(struct mgraph_function *fu, Uint8 r, Uint8 g, Uint8 b)
{
	
}

void
mgraph_compute_function(struct mgraph *gra, struct mgraph_function *func)
{
	
}

/* Destroy a function. */
void
mgraph_remove_function(struct mgraph *gra, struct mgraph_function *fu)
{
	TAILQ_REMOVE(&gra->functions, fu, functions);
	mgraph_free_function(fu);
}

void
mgraph_free_functions(struct mgraph *gra)
{
	struct mgraph_function *fu, *nfu;
	
	for (fu = TAILQ_FIRST(&gra->functions);
	     fu != TAILQ_LAST(&gra->functions, mgraph_functionq);
	     fu = nfu) {
		nfu = TAILQ_NEXT(fu, functions);
		mgraph_free_function(fu);
	}
	TAILQ_INIT(&gra->functions);
}

void
mgraph_destroy(void *p)
{
	struct mgraph *gra = p;

	mgraph_free_functions(gra);
	widget_destroy(gra);
}

#endif	/* FLOATING_POINT */
