/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Integer-only plotter widget. Used originally to implement a FPS meter.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/fixed_plotter.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

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

	if (flags & AG_FIXED_PLOTTER_HFILL) { WIDGET(fpl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FIXED_PLOTTER_VFILL) { WIDGET(fpl)->flags |= AG_WIDGET_VFILL; }
	fpl->flags |= flags;

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
}

void
AG_FixedPlotterSetRange(AG_FixedPlotter *fpl, AG_FixedPlotterValue range)
{
	AG_OBJECT_ISA(fpl, "AG_Widget:AG_FixedPlotter:*");
	fpl->yrange = range;
	AG_Redraw(fpl);
}

static void
KeyDown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_FIXEDPLOTTER_SELF();
	const int key = AG_INT(1);

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
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_FIXEDPLOTTER_SELF();
	const int xrel = AG_INT(3);
	const int yrel = AG_INT(4);
	const int state = AG_INT(5);

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
	AG_FixedPlotter *fpl = AG_FIXEDPLOTTER_SELF();

	fpl->flags |= AG_FIXED_PLOTTER_SCROLL;
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_FixedPlotter *fpl = AG_FIXEDPLOTTER_SELF();
	const int button = AG_INT(1);
	
	if (button != AG_MOUSE_LEFT)
		return;
	
	if (!AG_WidgetIsFocused(fpl))
		AG_WidgetFocus(fpl);

	fpl->flags &= ~(AG_FIXED_PLOTTER_SCROLL);
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
	AG_FixedPlotterValue oval;
	int x, y, ox = 0, oy;
	const int w = WIDTH(fpl);
	const int h = HEIGHT(fpl);
	Uint32 i, yOrigin;

	yOrigin = WIDGET(fpl)->h * fpl->yOrigin / 100;

	AG_DrawBoxRaised(fpl, &WIDGET(fpl)->r, &WCOLOR(fpl,BG_COLOR));

	if (fpl->flags & AG_FIXED_PLOTTER_XAXIS)
		AG_DrawLineH(fpl, 0, w-1, yOrigin+1,
		    &WCOLOR(fpl,LINE_COLOR));

	TAILQ_FOREACH(gi, &fpl->items, items) {
		if (fpl->xoffs > gi->nvals || fpl->xoffs < 0)
			continue;

		for (x = 2, ox = 0, i = fpl->xoffs;
		     ++i < gi->nvals && x < w;
		     ox = x, x += 2) {

			oval = gi->vals[i] * h / fpl->yrange;
			y = yOrigin - oval;
			if (i > 1) {
				oval = gi->vals[i-1] * h / fpl->yrange;
				oy = yOrigin - oval;
			} else {
				oy = yOrigin;
			}

			if (y < 0)  { y = 0; }
			if (oy < 0) { oy = 0; }
			if (y > HEIGHT(fpl))  { y = HEIGHT(fpl); }
			if (oy > HEIGHT(fpl)) { oy = HEIGHT(fpl); }

			switch (fpl->type) {
			case AG_FIXED_PLOTTER_POINTS:
				AG_PutPixel(fpl, x,y, &gi->color);
				break;
			case AG_FIXED_PLOTTER_LINES:
				AG_DrawLine(fpl, ox,oy, x,y, &gi->color);
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

	AG_OBJECT_ISA(fpl, "AG_Widget:AG_FixedPlotter:*");

 	gi = Malloc(sizeof(AG_FixedPlotterItem));
	Strlcpy(gi->name, name, sizeof(gi->name));
	AG_ColorRGB_8(&gi->color, r,g,b);
	gi->nvals = 0;
	gi->vals = Malloc(NITEMS_INIT*sizeof(AG_FixedPlotterValue));
	gi->maxvals = NITEMS_INIT;
	gi->fpl = fpl;
	gi->limit = limit>0 ? limit : (0xffffffff-1);

	AG_ObjectLock(fpl);
	TAILQ_INSERT_HEAD(&fpl->items, gi, items);
	AG_Redraw(fpl);
	AG_ObjectUnlock(fpl);

	return (gi);
}

void
AG_FixedPlotterDatum(AG_FixedPlotterItem *gi, AG_FixedPlotterValue val)
{
	AG_FixedPlotter *fpl = gi->fpl;

	AG_OBJECT_ISA(fpl, "AG_Widget:AG_FixedPlotter:*");
	AG_ObjectLock(fpl);
	
	gi->vals[gi->nvals] = val;

	if (gi->nvals+1 >= gi->limit) {
		memmove(gi->vals, gi->vals+1,
		        gi->nvals*sizeof(AG_FixedPlotterValue));
		fpl->flags &= ~(AG_FIXED_PLOTTER_SCROLL);
	} else {
		if (gi->nvals+1 >= gi->maxvals) {
			gi->vals = Realloc(gi->vals,
			    (gi->maxvals+NITEMS_GROW) *
			    sizeof(AG_FixedPlotterValue));
			gi->maxvals += NITEMS_GROW;
		}
		gi->nvals++;
	}
	
	AG_Redraw(fpl);
	AG_ObjectUnlock(fpl);
}

void
AG_FixedPlotterFreeItems(AG_FixedPlotter *fpl)
{
	AG_FixedPlotterItem *git, *nextgit;
	
	AG_OBJECT_ISA(fpl, "AG_Widget:AG_FixedPlotter:*");
	AG_ObjectLock(fpl);

	for (git = TAILQ_FIRST(&fpl->items);
	     git != TAILQ_END(&fpl->items);
	     git = nextgit) {
		nextgit = TAILQ_NEXT(git, items);
		Free(git->vals);
		Free(git);
	}
	TAILQ_INIT(&fpl->items);

	AG_Redraw(fpl);
	AG_ObjectUnlock(fpl);
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

void
AG_FixedPlotterScroll(AG_FixedPlotter *_Nonnull fpl, int i)
{
	AG_OBJECT_ISA(fpl, "AG_Widget:AG_FixedPlotter:*");
	AG_ObjectLock(fpl);

	if (fpl->flags & AG_FIXED_PLOTTER_SCROLL)
		fpl->xoffs += i;

	AG_ObjectUnlock(fpl);
}

AG_WidgetClass agFixedPlotterClass = {
	{
		"Agar(Widget:FixedPlotter)",
		sizeof(AG_FixedPlotter),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGET */
