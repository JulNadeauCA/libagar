/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "fixed_plotter.h"

#include "window.h"
#include "primitive.h"

const AG_WidgetOps agFixedPlotterOps = {
	{
		"AG_Widget:AG_FixedPlotter",
		sizeof(AG_FixedPlotter),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_FixedPlotterDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_FixedPlotterDraw,
	AG_FixedPlotterScale
};

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void	keydown(AG_Event *);
static void	mousemotion(AG_Event *);
static void	mousebuttondown(AG_Event *);
static void	mousebuttonup(AG_Event *);

AG_FixedPlotter *
AG_FixedPlotterNew(void *parent, enum ag_fixed_plotter_type type, Uint flags)
{
	AG_FixedPlotter *fpl;

	fpl = Malloc(sizeof(AG_FixedPlotter), M_OBJECT);
	AG_FixedPlotterInit(fpl, type, flags);
	AG_ObjectAttach(parent, fpl);
	if (flags & AG_FIXED_PLOTTER_FOCUS) {
		AG_WidgetFocus(fpl);
	}
	return (fpl);
}

void
AG_FixedPlotterInit(AG_FixedPlotter *fpl, enum ag_fixed_plotter_type type,
    Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_FIXED_PLOTTER_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_FIXED_PLOTTER_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(fpl, &agFixedPlotterOps, wflags);
	fpl->type = type;
	fpl->flags = flags;
	fpl->xoffs = 0;
	fpl->origin_y = 50;
	fpl->yrange = 100;
	TAILQ_INIT(&fpl->items);

	AG_SetEvent(fpl, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(fpl, "window-keydown", keydown, NULL);
	AG_SetEvent(fpl, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(fpl, "window-mousebuttondown", mousebuttondown, NULL);
}

void
AG_FixedPlotterSetRange(AG_FixedPlotter *fpl, AG_FixedPlotterValue range)
{
	fpl->yrange = range;
}

static void
keydown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	SDLKey key = AG_SDLKEY(1);

	switch (key) {
	case SDLK_0:
		fpl->xoffs = 0;
		break;
	case SDLK_LEFT:
		if ((fpl->xoffs -= 10) < 0)
			fpl->xoffs = 0;
		break;
	case SDLK_RIGHT:
		fpl->xoffs += 10;
		break;
	default:
		break;
	}
}

static void
mousemotion(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int state = AG_INT(5);

	if ((state & SDL_BUTTON_LMASK) == 0)
		return;

	if ((fpl->xoffs -= xrel) < 0)
		fpl->xoffs = 0;

	fpl->origin_y += yrel;
	if (fpl->origin_y < 0)
		fpl->origin_y = 0;
	if (fpl->origin_y > AGWIDGET(fpl)->h)
		fpl->origin_y = AGWIDGET(fpl)->h;
}

static void
mousebuttonup(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();

	fpl->flags |= AG_FIXED_PLOTTER_SCROLL;
}

static void
mousebuttondown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	int button = AG_INT(1);
	
	if (button != SDL_BUTTON_LEFT)
		return;

	fpl->flags &= ~(AG_FIXED_PLOTTER_SCROLL);
	AG_WidgetFocus(fpl);
}

void
AG_FixedPlotterScale(void *p, int w, int h)
{
	AG_FixedPlotter *fpl = p;

	if (w == -1 && h == -1) {
		AGWIDGET(fpl)->w = 100;
		AGWIDGET(fpl)->h = 80;
	}
}

void
AG_FixedPlotterDraw(void *p)
{
	AG_FixedPlotter *fpl = p;
	AG_FixedPlotterItem *gi;
	int x, y, ox = 0, oy;
	Uint32 i, origin_y;
	AG_FixedPlotterValue oval;

	origin_y = AGWIDGET(fpl)->h * fpl->origin_y / 100;

	agPrim.box(fpl,
	    0, 0,
	    AGWIDGET(fpl)->w, AGWIDGET(fpl)->h,
	    0,
	    AG_COLOR(GRAPH_BG_COLOR));

	if (fpl->flags & AG_FIXED_PLOTTER_XAXIS) {
		agPrim.hline(fpl,
		    0,
		    AGWIDGET(fpl)->w-1,
		    origin_y + 1,
		    AG_COLOR(GRAPH_XAXIS_COLOR));
	}

	TAILQ_FOREACH(gi, &fpl->items, items) {
		if (fpl->xoffs > gi->nvals || fpl->xoffs < 0)
			continue;

		for (x = 2, ox = 0, i = fpl->xoffs;
		     ++i < gi->nvals && x < AGWIDGET(fpl)->w;
		     ox = x, x += 2) {

			oval = gi->vals[i] * AGWIDGET(fpl)->h / fpl->yrange;
			y = origin_y - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * AGWIDGET(fpl)->h /
				       fpl->yrange;
				oy = origin_y - oval;
			} else {
				oy = origin_y;
			}

			if (y < 0)
				y = 0;
			if (oy < 0)
				oy = 0;
			if (y > AGWIDGET(fpl)->h)
				y = AGWIDGET(fpl)->h;
			if (oy > AGWIDGET(fpl)->h)
				oy = AGWIDGET(fpl)->h;

			switch (fpl->type) {
			case AG_FIXED_PLOTTER_POINTS:
				AG_WidgetPutPixel(fpl, x, y, gi->color);
				break;
			case AG_FIXED_PLOTTER_LINES:
				agPrim.line(fpl, ox, oy, x, y,
				    gi->color);
				break;
			}
		}
	}
}

AG_FixedPlotterItem *
AG_FixedPlotterCurve(AG_FixedPlotter *fpl, const char *name,
    Uint8 r, Uint8 g, Uint8 b, Uint32 limit)
{
	AG_FixedPlotterItem *gi;

 	gi = Malloc(sizeof(AG_FixedPlotterItem), M_WIDGET);
	strlcpy(gi->name, name, sizeof(gi->name));
	gi->color = SDL_MapRGB(agVideoFmt, r, g, b);
	gi->vals = Malloc(NITEMS_INIT * sizeof(AG_FixedPlotterValue), M_WIDGET);
	gi->maxvals = NITEMS_INIT;
	gi->nvals = 0;
	gi->fpl = fpl;
	gi->limit = limit>0 ? limit : (0xffffffff-1);
	TAILQ_INSERT_HEAD(&fpl->items, gi, items);
	return (gi);
}

void
AG_FixedPlotterDatum(AG_FixedPlotterItem *gi, AG_FixedPlotterValue val)
{
	gi->vals[gi->nvals] = val;

	if (gi->nvals+1 >= gi->limit) {
		memmove(gi->vals, gi->vals+1,
		        gi->nvals*sizeof(AG_FixedPlotterValue));
		gi->fpl->flags &= ~(AG_FIXED_PLOTTER_SCROLL);
	} else {
		if (gi->nvals+1 >= gi->maxvals) {
			gi->vals = Realloc(gi->vals,
			    (gi->maxvals+NITEMS_GROW) *
			    sizeof(AG_FixedPlotterValue));
			gi->maxvals += NITEMS_GROW;
		}
		gi->nvals++;
	}
}

void
AG_FixedPlotterScroll(AG_FixedPlotter *fpl, int i)
{
	if (fpl->flags & AG_FIXED_PLOTTER_SCROLL)
		fpl->xoffs += i;
}

void
AG_FixedPlotterFreeItems(AG_FixedPlotter *fpl)
{
	AG_FixedPlotterItem *git, *nextgit;
	
	for (git = TAILQ_FIRST(&fpl->items);
	     git != TAILQ_END(&fpl->items);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		Free(git->vals, M_WIDGET);
		Free(git, M_WIDGET);
	}
	TAILQ_INIT(&fpl->items);
}

void
AG_FixedPlotterDestroy(void *p)
{
	AG_FixedPlotter *fpl = p;

	AG_FixedPlotterFreeItems(fpl);
	AG_WidgetDestroy(fpl);
}
