/*	$Csoft: box.c,v 1.13 2005/03/10 09:43:34 vedge Exp $	*/

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
#include <engine/view.h>

#include "box.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static AG_WidgetOps box_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_BoxDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,		/* draw */
	AG_BoxScale
};

static AG_WidgetOps box_ops_visframe = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_BoxDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_BoxDraw,
	AG_BoxScale
};

AG_Box *
AG_BoxNew(void *parent, enum ag_box_type type, int flags)
{
	AG_Box *bo;

	bo = Malloc(sizeof(AG_Box), M_OBJECT);
	AG_BoxInit(bo, type, flags);
	AG_ObjectAttach(parent, bo);
	return (bo);
}

void
AG_BoxInit(AG_Box *bo, enum ag_box_type type, int flags)
{
	AG_WidgetInit(bo, "box", (flags & AG_BOX_FRAME) ?
	    &box_ops_visframe : &box_ops, 0);

	bo->type = type;
	bo->depth = -1;

	if (flags & AG_BOX_WFILL) { AGWIDGET(bo)->flags |= AG_WIDGET_WFILL; }
	if (flags & AG_BOX_HFILL) { AGWIDGET(bo)->flags |= AG_WIDGET_HFILL; }

	bo->padding = 2;
	bo->spacing = 1;
	bo->homogenous = (flags & AG_BOX_HOMOGENOUS);
	pthread_mutex_init(&bo->lock, &agRecursiveMutexAttr);
}

void
AG_BoxDestroy(void *p)
{
	AG_Box *box = p;

	pthread_mutex_destroy(&box->lock);
	AG_WidgetDestroy(box);
}

void
AG_BoxDraw(void *p)
{
	AG_Box *bo = p;

	agPrim.box(bo, 0, 0, AGWIDGET(bo)->w, AGWIDGET(bo)->h, bo->depth,
	    AG_COLOR(FRAME_COLOR));
}

void
AG_BoxScale(void *p, int w, int h)
{
	AG_Box *bo = p;
	AG_Widget *wid;
	int x = bo->padding, y = bo->padding;
	int nwidgets = 0;
	int totfixed = 0;

	pthread_mutex_lock(&bo->lock);
	
	/* Count the child widgets. */
	AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {
		AGWIDGET_OPS(wid)->scale(wid, -1, -1);

		switch (bo->type) {
		case AG_BOX_HORIZ:
			if ((wid->flags & AG_WIDGET_WFILL) == 0)
				totfixed += wid->w + bo->spacing;
			break;
		case AG_BOX_VERT:
			if ((wid->flags & AG_WIDGET_HFILL) == 0)
				totfixed += wid->h + bo->spacing;
			break;
		}
		nwidgets++;
	}
	if (nwidgets > 0)
		totfixed -= bo->spacing;

	if (w == -1 && h == -1) {				/* Size hint */
		int maxw = 0, maxh = 0;
		int dw, dh;

		AGWIDGET(bo)->w = bo->padding*2;
		AGWIDGET(bo)->h = bo->padding*2;

		/* Reserve enough space to hold widgets and spacing/padding. */
		AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {
			AGWIDGET_OPS(wid)->scale(wid, -1, -1);
			if (wid->w > maxw) maxw = wid->w;
			if (wid->h > maxh) maxh = wid->h;

			switch (bo->type) {
			case AG_BOX_HORIZ:
				if ((dh = maxh + bo->padding*2) >
				    AGWIDGET(bo)->h) {
					AGWIDGET(bo)->h = dh;
				}
				AGWIDGET(bo)->w += wid->w + bo->spacing;
				break;
			case AG_BOX_VERT:
				if ((dw = maxw + bo->padding*2) >
				    AGWIDGET(bo)->w) {
					AGWIDGET(bo)->w = dw;
				}
				AGWIDGET(bo)->h += wid->h + bo->spacing;
				break;
			}
		}
		if (nwidgets > 0) {
			switch (bo->type) {
			case AG_BOX_HORIZ:
				AGWIDGET(bo)->w -= bo->spacing;
				break;
			case AG_BOX_VERT:
				AGWIDGET(bo)->h -= bo->spacing;
				break;
			}
		}
		goto out;
	}

	if (bo->homogenous) {
		int max = 0, excedent = 0, nexcedent = 0;

		/* Divide the space among widgets. */
		switch (bo->type) {
		case AG_BOX_HORIZ:
			if (nwidgets > 0) {
				max = (w - bo->padding*2 -
				       bo->spacing*(nwidgets-1));
				max /= nwidgets;
			} else {
				max = w - bo->padding*2;
			}
			break;
		case AG_BOX_VERT:
			if (nwidgets > 0) {
				max = (h - bo->padding*2 -
				       bo->spacing*(nwidgets-1));
				max /= nwidgets;
			} else {
				max = h - bo->padding*2;
			}
			break;
		}
		AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {
			wid->x = x;
			wid->y = y;
			
			switch (bo->type) {
			case AG_BOX_HORIZ:
				AGWIDGET_OPS(wid)->scale(wid, -1, -1);
				wid->w = max;
				wid->h = h - bo->padding*2;
				AGWIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
				x += wid->w + bo->spacing;
				break;
			case AG_BOX_VERT:
				wid->h = max;
				wid->w = w - bo->padding*2;
				y += wid->h + bo->spacing;
				break;
			}
		}
		goto out;
	}

	AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {	/* Fixed/[wh]fill */
		wid->x = x;
		wid->y = y;

		/*
		 * Position fixed-size widgets and allocate the remaining
		 * space to [wh]fill widgets.
		 */
		switch (bo->type) {
		case AG_BOX_HORIZ:
			if (wid->flags & AG_WIDGET_WFILL) {
				wid->w = w - totfixed - bo->padding*2;
			}
			if (wid->flags & AG_WIDGET_HFILL) {
				wid->h = h - bo->padding*2;
			}
			x += wid->w + bo->spacing;
			break;
		case AG_BOX_VERT:
			if (wid->flags & AG_WIDGET_WFILL) {
				wid->w = w - bo->padding*2; 
			}
			if (wid->flags & AG_WIDGET_HFILL) {
				wid->h = h - totfixed - bo->padding*2;
			}
			y += wid->h + bo->spacing;
			break;
		}
		AGWIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
	}
out:
	pthread_mutex_unlock(&bo->lock);
}

void
AG_BoxSetHomogenous(AG_Box *bo, int homogenous)
{
	pthread_mutex_lock(&bo->lock);
	bo->homogenous = homogenous;
	pthread_mutex_unlock(&bo->lock);
}

void
AG_BoxSetPadding(AG_Box *bo, int padding)
{
	pthread_mutex_lock(&bo->lock);
	bo->padding = padding;
	pthread_mutex_unlock(&bo->lock);
}

void
AG_BoxSetSpacing(AG_Box *bo, int spacing)
{
	pthread_mutex_lock(&bo->lock);
	bo->spacing = spacing;
	pthread_mutex_unlock(&bo->lock);
}

void
AG_BoxSetDepth(AG_Box *bo, int depth)
{
	pthread_mutex_lock(&bo->lock);
	bo->depth = depth;
	pthread_mutex_unlock(&bo->lock);
}
