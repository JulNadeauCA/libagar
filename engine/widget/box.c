/*	$Csoft: box.c,v 1.8 2004/03/18 21:27:48 vedge Exp $	*/

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

static struct widget_ops box_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		box_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,		/* draw */
	box_scale
};

enum {
	FRAME_COLOR
};

struct box *
box_new(void *parent, enum box_type type, int flags)
{
	struct box *bo;

	bo = Malloc(sizeof(struct box), M_OBJECT);
	box_init(bo, type, flags);
	object_attach(parent, bo);
	return (bo);
}

void
box_init(struct box *bo, enum box_type type, int flags)
{
	widget_init(bo, "box", &box_ops, 0);
	widget_map_color(bo, FRAME_COLOR, "frame", 80, 80, 80, 255);

	bo->type = type;

	if (flags & BOX_WFILL)
		WIDGET(bo)->flags |= WIDGET_WFILL;
	if (flags & BOX_HFILL)
		WIDGET(bo)->flags |= WIDGET_HFILL;

	bo->padding = 4;
	bo->spacing = 2;
	bo->homogenous = (flags & BOX_HOMOGENOUS);
	pthread_mutex_init(&bo->lock, &recursive_mutexattr);
}

void
box_destroy(void *p)
{
	struct box *box = p;

	pthread_mutex_destroy(&box->lock);
	widget_destroy(box);
}

#if 0
void
box_draw(void *p)
{
	struct box *bo = p;

	primitives.box(bo, 0, 0, WIDGET(bo)->w, WIDGET(bo)->h, -1, FRAME_COLOR);
}
#endif

void
box_scale(void *p, int w, int h)
{
	struct box *bo = p;
	struct widget *wid;
	int x = bo->padding, y = bo->padding;
	int nwidgets = 0, nfill = 0, nfixed = 0;
	int totfixed = 0;

	pthread_mutex_lock(&bo->lock);
	
	/* Count the child widgets. */
	OBJECT_FOREACH_CHILD(wid, bo, widget) {
		WIDGET_OPS(wid)->scale(wid, -1, -1);
		switch (bo->type) {
		case BOX_HORIZ:
			if (wid->flags & WIDGET_WFILL) {
				nfill++;
			} else {
				totfixed += wid->w;
				nfixed++;
			}
			break;
		case BOX_VERT:
			if (wid->flags & WIDGET_HFILL) {
				nfill++;
			} else {
				totfixed += wid->h;
				nfixed++;
			}
			break;
		}
		nwidgets++;
	}

	if (w == -1 && h == -1) {				/* Size hint */
		int maxw = 0, maxh = 0;

		WIDGET(bo)->w = bo->padding*2;
		WIDGET(bo)->h = bo->padding*2;

		/* Reserve enough space to hold widgets and spacing/padding. */
		OBJECT_FOREACH_CHILD(wid, bo, widget) {
			WIDGET_OPS(wid)->scale(wid, -1, -1);

			if (wid->w > maxw)
				maxw = wid->w;
			if (wid->h > maxh)
				maxh = wid->h;

			switch (bo->type) {
			case BOX_HORIZ:
				if ((maxh + bo->padding*2) > WIDGET(bo)->h) {
					WIDGET(bo)->h = maxh + bo->padding*2;
				}
				WIDGET(bo)->w += wid->w + bo->spacing;
				break;
			case BOX_VERT:
				if ((maxw + bo->padding*2) > WIDGET(bo)->w) {
					WIDGET(bo)->w = maxw + bo->padding*2;
				}
				WIDGET(bo)->h += wid->h + bo->spacing;
				break;
			}
		}
		if (nwidgets > 0) {
			switch (bo->type) {
			case BOX_HORIZ:
				WIDGET(bo)->w -= bo->spacing;
				break;
			case BOX_VERT:
				WIDGET(bo)->h -= bo->spacing;
				break;
			}
		}
		goto out;
	}

	if (bo->homogenous) {
		int max = 0, excedent = 0, nexcedent = 0;

		/* Divide the space among widgets. */
		switch (bo->type) {
		case BOX_HORIZ:
			if (nwidgets > 0) {
				max = (w - bo->padding*2 -
				       bo->spacing*(nwidgets-1));
				max /= nwidgets;
			} else {
				max = w - bo->padding*2;
			}
			break;
		case BOX_VERT:
			if (nwidgets > 0) {
				max = (h - bo->padding*2 -
				       bo->spacing*(nwidgets-1));
				max /= nwidgets;
			} else {
				max = h - bo->padding*2;
			}
			break;
		}
		OBJECT_FOREACH_CHILD(wid, bo, widget) {
			wid->x = x;
			wid->y = y;
			
			switch (bo->type) {
			case BOX_HORIZ:
				WIDGET_OPS(wid)->scale(wid, -1, -1);
				if (wid->w > max) {
					wid->flags |= WIDGET_EXCEDENT;
					excedent += wid->w - max;
					nexcedent++;
				} else {
					wid->w = max;
				}
				wid->h = h - bo->padding*2;
				WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
				x += wid->w + bo->spacing;
				break;
			case BOX_VERT:
				if (wid->h > max) {
					wid->flags |= WIDGET_EXCEDENT;
					excedent += wid->h - max;
					nexcedent++;
				} else {
					wid->h = max;
				}
				wid->w = w - bo->padding*2;
				y += wid->h + bo->spacing;
				break;
			}
		}

		/* Adjust for widgets that are too big. */
		x = bo->padding;
		y = bo->padding;
		OBJECT_FOREACH_CHILD(wid, bo, widget) {
			wid->x = x;
			wid->y = y;
			switch (bo->type) {
			case BOX_HORIZ:
				if ((wid->flags & WIDGET_EXCEDENT) == 0 &&
				    nexcedent < nwidgets) {
					wid->w -= excedent/(nwidgets-nexcedent);
				}
				x += wid->w + bo->spacing;
				break;
			case BOX_VERT:
				if ((wid->flags & WIDGET_EXCEDENT) == 0 &&
				    nexcedent < nwidgets) {
					wid->h -= excedent/(nwidgets-nexcedent);
				}
				y += wid->h + bo->spacing;
				break;
			}
		}
		goto out;
	}

	OBJECT_FOREACH_CHILD(wid, bo, widget) {		/* Fixed/[wh]fill */
		wid->x = x;
		wid->y = y;

		/*
		 * Position fixed-size widgets and allocate the remaining
		 * space to [wh]fill widgets.
		 */
		switch (bo->type) {
		case BOX_HORIZ:
			if (wid->flags & WIDGET_WFILL) {
				wid->w = w - totfixed -
				    (nfixed-1)*bo->spacing*2;
			}
			if (wid->flags & WIDGET_HFILL) {
				wid->h = h - bo->padding*2;
			}
			x += wid->w + bo->spacing;
			break;
		case BOX_VERT:
			if (wid->flags & WIDGET_WFILL) {
				wid->w = w - bo->padding*2; 
			}
			if (wid->flags & WIDGET_HFILL) {
				wid->h = h - totfixed -
				    (nfixed-1)*bo->spacing*2;
			}
			y += wid->h + bo->spacing;
			break;
		}
		WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
	}
out:
	pthread_mutex_unlock(&bo->lock);
}

void
box_set_homogenous(struct box *bo, int homogenous)
{
	pthread_mutex_lock(&bo->lock);
	bo->homogenous = homogenous;
	pthread_mutex_unlock(&bo->lock);
}

void
box_set_padding(struct box *bo, int padding)
{
	pthread_mutex_lock(&bo->lock);
	bo->padding = padding;
	pthread_mutex_unlock(&bo->lock);
}

void
box_set_spacing(struct box *bo, int spacing)
{
	pthread_mutex_lock(&bo->lock);
	bo->spacing = spacing;
	pthread_mutex_unlock(&bo->lock);
}
