/*
 * Copyright (c) 2002-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "fixed_plotter.h"
#include "window.h"
#include "primitive.h"

enum {
	NITEMS_INIT =	32,
	NITEMS_GROW =	16
};

static void KeyDown(AG_Event *);
static void MouseMotion(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void MouseButtonUp(AG_Event *);

AG_FixedPlotter *
AG_FixedPlotterNew(void *parent, enum ag_fixed_plotter_type type, Uint flags)
{
	AG_FixedPlotter *fpl;

	fpl = Malloc(sizeof(AG_FixedPlotter));
	AG_ObjectInit(fpl, &agFixedPlotterClass);
	fpl->type = type;
	fpl->flags |= flags;

	if (flags & AG_FIXED_PLOTTER_HFILL) { AG_ExpandHoriz(fpl); }
	if (flags & AG_FIXED_PLOTTER_VFILL) { AG_ExpandVert(fpl); }

	AG_ObjectAttach(parent, fpl);
	return (fpl);
}

static void
Init(void *obj)
{
	AG_FixedPlotter *fpl = obj;
	
	WIDGET(fpl)->flags |= AG_WIDGET_FOCUSABLE;

	fpl->type = AG_FIXED_PLOTTER_LINES;
	fpl->flags = 0;
	fpl->xoffs = 0;
	fpl->yOrigin = 50;
	fpl->yrange = 100;
	TAILQ_INIT(&fpl->items);

	AG_SetEvent(fpl, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(fpl, "key-down", KeyDown, NULL);
	AG_SetEvent(fpl, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(fpl, "mouse-button-down", MouseButtonDown, NULL);

#ifdef AG_DEBUG
	AG_BindUint(fpl, "type", &fpl->type);
	AG_BindUint(fpl, "flags", &fpl->flags);
	AG_BindSint16(fpl, "yrange", &fpl->yrange);
	AG_BindSint16(fpl, "xoffs", &fpl->xoffs);
	AG_BindInt(fpl, "yOrigin", &fpl->yOrigin);
#endif
}

void
AG_FixedPlotterSetRange(AG_FixedPlotter *fpl, AG_FixedPlotterValue range)
{
	AG_ObjectLock(fpl);
	fpl->yrange = range;
	AG_ObjectUnlock(fpl);
	AG_Redraw(fpl);
}

static void
KeyDown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	int key = AG_INT(1);

	switch (key) {
	case AG_KEY_0:
		fpl->xoffs = 0;
		AG_Redraw(fpl);
		break;
	case AG_KEY_LEFT:
		if ((fpl->xoffs -= 10) < 0) {
			fpl->xoffs = 0;
		}
		AG_Redraw(fpl);
		break;
	case AG_KEY_RIGHT:
		fpl->xoffs += 10;
		AG_Redraw(fpl);
		break;
	default:
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int state = AG_INT(5);

	if ((state & AG_MOUSE_LMASK) == 0)
		return;

	if ((fpl->xoffs -= xrel) < 0)
		fpl->xoffs = 0;

	fpl->yOrigin += yrel;
	if (fpl->yOrigin < 0)
		fpl->yOrigin = 0;
	if (fpl->yOrigin > WIDGET(fpl)->h)
		fpl->yOrigin = WIDGET(fpl)->h;

	if (xrel != 0 || yrel != 0)
		AG_Redraw(fpl);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();

	fpl->flags |= AG_FIXED_PLOTTER_SCROLL;
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_SELF();
	int button = AG_INT(1);
	
	if (button != AG_MOUSE_LEFT)
		return;

	fpl->flags &= ~(AG_FIXED_PLOTTER_SCROLL);
	AG_WidgetFocus(fpl);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	r->w = 256;
	r->h = 128;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	return (a->w > 2 && a->h > 4) ? 0 : -1;
}

static void
Draw(void *obj)
{
	AG_FixedPlotter *fpl = obj;
	AG_FixedPlotterItem *gi;
	int x, y, ox = 0, oy;
	Uint32 i, yOrigin;
	AG_FixedPlotterValue oval;

	yOrigin = WIDGET(fpl)->h * fpl->yOrigin / 100;
	STYLE(fpl)->FixedPlotterBackground(fpl,
	    (fpl->flags & AG_FIXED_PLOTTER_XAXIS), yOrigin);

	TAILQ_FOREACH(gi, &fpl->items, items) {
		if (fpl->xoffs > gi->nvals || fpl->xoffs < 0)
			continue;

		for (x = 2, ox = 0, i = fpl->xoffs;
		     ++i < gi->nvals && x < WIDGET(fpl)->w;
		     ox = x, x += 2) {

			oval = gi->vals[i] * WIDGET(fpl)->h / fpl->yrange;
			y = yOrigin - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * WIDGET(fpl)->h /
				       fpl->yrange;
				oy = yOrigin - oval;
			} else {
				oy = yOrigin;
			}

			if (y < 0) { y = 0; }
			if (oy < 0) { oy = 0; }
			if (y > WIDGET(fpl)->h) { y = WIDGET(fpl)->h; }
			if (oy > WIDGET(fpl)->h) { oy = WIDGET(fpl)->h; }

			switch (fpl->type) {
			case AG_FIXED_PLOTTER_POINTS:
				AG_PutPixel(fpl, x, y, gi->color);
				break;
			case AG_FIXED_PLOTTER_LINES:
				AG_DrawLine(fpl, ox, oy, x, y, gi->color);
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

 	gi = Malloc(sizeof(AG_FixedPlotterItem));
	Strlcpy(gi->name, name, sizeof(gi->name));
	gi->color = AG_ColorRGB(r,g,b);
	gi->vals = Malloc(NITEMS_INIT*sizeof(AG_FixedPlotterValue));
	gi->maxvals = NITEMS_INIT;
	gi->nvals = 0;
	gi->fpl = fpl;
	gi->limit = limit>0 ? limit : (0xffffffff-1);

	AG_ObjectLock(fpl);
	TAILQ_INSERT_HEAD(&fpl->items, gi, items);
	AG_ObjectUnlock(fpl);
	
	AG_Redraw(fpl);
	return (gi);
}

void
AG_FixedPlotterDatum(AG_FixedPlotterItem *gi, AG_FixedPlotterValue val)
{
	AG_ObjectLock(gi->fpl);
	
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
	
	AG_ObjectUnlock(gi->fpl);
	AG_Redraw(gi->fpl);
}

void
AG_FixedPlotterFreeItems(AG_FixedPlotter *fpl)
{
	AG_FixedPlotterItem *git, *nextgit;
	
	AG_ObjectLock(fpl);
	for (git = TAILQ_FIRST(&fpl->items);
	     git != TAILQ_END(&fpl->items);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		Free(git->vals);
		Free(git);
	}
	TAILQ_INIT(&fpl->items);
	AG_ObjectUnlock(fpl);
	AG_Redraw(fpl);
}

static void
Destroy(void *obj)
{
	AG_FixedPlotter *fpl = obj;
	AG_FixedPlotterItem *git, *nextgit;

	for (git = TAILQ_FIRST(&fpl->items);
	     git != TAILQ_END(&fpl->items);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		Free(git->vals);
		Free(git);
	}
}

AG_WidgetClass agFixedPlotterClass = {
	{
		"Agar(Widget:FixedPlotter)",
		sizeof(AG_FixedPlotter),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
