/*	$Csoft$	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "text.h"
#include "widget.h"
#include "window.h"

static const struct object_ops region_ops = {
	region_destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* onattach */
	NULL,		/* ondetach */
	region_attach,
	region_detach
};

struct region *
region_new(void *parent, enum region_salign salign, int seg_winpct,
    enum region_walign walign, int wspacing)
{
	struct region *seg;

	seg = emalloc(sizeof(struct region));
	region_init(seg, segtype, seg_winpct);

	if (OBJECT_TYPE(parent, "window")) {
		struct window *win = parent;

		pthread_mutex_lock(&win->lock);
		window_attach(win, seg);
		pthread_mutex_unlock(&win->lock);
	} else if (OBJECT_TYPE(parent, "window-region")) {
		struct region *pseg = parent;
	
		pthread_mutex_lock(&pseg->win->lock);
		region_attach(pseg, seg);
		pthread_mutex_unlock(&pseg->win->lock);
	} else {
		fatal("cannot attach to %s\n", OBJECT(parent)->type);
	}
	return (seg);
}

void
region_init(struct region *seg, enum region_salign salign, int seg_winpct,
    enum region_walign walign, int wspacing)
{
	static int curseg = 0;
	char *segname;

	segname = object_name("window-region", curseg++);
	object_init(&seg->obj, "window-region", segname, NULL, NULL,
	    &region_ops);
	free(segname);
	seg->win = NULL;
	seg->type = segtype;

	seg->reqpct = reqpct;
	seg->x = 0;
	seg->y = 0;
	seg->w = 0;
	seg->h = 0;
	seg->lastx = 0;
	seg->lasty = 0;
	SLIST_INIT(&seg->childsh);
}

void
region_destroy(void *p)
{
	struct region *seg = p;

	dprintf("free %s\n", OBJECT(seg)->name);
}

/*
 * Attach a widget or another region inside this region.
 * Window must be locked.
 */
void
region_attach(void *parent, void *child)
{
	struct region *pseg = parent;
	struct region_child *segchild;

	OBJECT_ASSERT(parent, "window-region");
	
	segchild = emalloc(sizeof(struct region_child));

	if (strcmp("widget", OBJECT(child)->type) == 0) {
		struct widget *wid = child;

		dprintf("attach widget %s to region*\n", OBJECT(wid)->name);

		/*
		 * Attach the widget to the parent region.
		 * The widget is positioned only later.
		 */
		wid->win = pseg->win;
		segchild->ref.seg = NULL;
		segchild->ref.wid = wid;
		if (OBJECT_OPS(wid)->onattach != NULL) {
			OBJECT_OPS(wid)->onattach(pseg->win, wid);
		}
		wid->win->redraw++;
	} else if (strcmp("window-region", OBJECT(child)->type) == 0) {
		struct region *seg = child;

		dprintf("attach region %s to region*\n", OBJECT(seg)->name);

		/*
		 * Attach the child region to the parent region.
		 * Unlike widgets, the child region is positioned immediately.
		 */
		seg->win = pseg->win;
		switch (seg->type) {
		case SEGMENT_HORIZ:
			seg->w = pseg->w;
			seg->h = seg->reqpct * pseg->w / 100;
			seg->x = pseg->lastx;
			seg->y = pseg->lasty;
			pseg->lasty += seg->h;
			break;
		case SEGMENT_VERT:
			seg->w = seg->reqpct * pseg->h / 100;
			seg->h = pseg->h;
			seg->x = pseg->lastx;
			seg->y = pseg->lasty;
			pseg->lastx += seg->w;
			break;
		}
		segchild->ref.seg = seg;
		segchild->ref.wid = NULL;
		seg->win->redraw++;
		dprintf("%s subregion of %dx%d at %d,%d (%d%% of pseg)\n",
		    (seg->type == SEGMENT_HORIZ) ? "horiz" : "vert",
		    seg->w, seg->h,
		    seg->x, seg->y, seg->reqpct);
	} else {
		fatal("cannot contain %s\n", OBJECT(child)->name);
	}

	SLIST_INSERT_HEAD(&pseg->childsh, segchild, childs);

	/* Update widget positions. */
	window_update(pseg->win);
}

/*
 * Detach a widget or another region from this region.
 * Window must be locked.
 */
void
region_detach(void *parent, void *child)
{
	OBJECT_ASSERT(parent, "window-region");

	if (strcmp("widget", OBJECT(child)->type) == 0) {
		struct widget *wid = child;

		/* Detach the widget from its parent region. */
		dprintf("detach a widget\n");
		if (OBJECT_OPS(wid)->ondetach != NULL) {
			OBJECT_OPS(wid)->ondetach(wid->win, wid);
		}
		wid->win->redraw++;
	} else if (strcmp("widget", OBJECT(child)->type) == 0) {
		dprintf("detach a region\n");
	}
}
