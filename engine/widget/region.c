/*	$Csoft: region.c,v 1.1 2002/05/19 14:30:24 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
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
region_new(void *parent, enum widget_align walign, int rx, int ry, int rw,
    int rh, int spacing)
{
	struct region *reg;
	struct window *win = parent;

	OBJECT_ASSERT(parent, "window");

	reg = emalloc(sizeof(struct region));
	region_init(reg, walign, rx, ry, rw, rh, spacing);

	pthread_mutex_lock(&win->lock);
	window_attach(win, reg);
	pthread_mutex_unlock(&win->lock);

	return (reg);
}

void
region_init(struct region *reg, enum widget_align walign, int rx, int ry,
    int rw, int rh, int spacing)
{
	static int curreg = 0;
	char *regname;

	regname = object_name("window-region", curreg++);
	object_init(&reg->obj, "window-region", regname, NULL, NULL,
	    &region_ops);
	free(regname);

	reg->walign = walign;
	reg->rx = rx;
	reg->ry = ry;
	reg->rw = NULL;
	reg->rh = NULL;
	reg->x = 0;
	reg->y = 0;
	reg->w = 0;
	reg->h = 0;
	reg->spacing = spacing;
	reg->win = NULL;
	TAILQ_INIT(&reg->widgetsh);
}

void
region_destroy(void *p)
{
	struct region *reg = p;

	dprintf("free %s\n", OBJECT(reg)->name);
}

/*
 * Attach a widget to this region.
 * Window must be locked.
 */
void
region_attach(void *parent, void *child)
{
	struct region *reg = parent;
	struct widget *wid = child;

	OBJECT_ASSERT(parent, "window-region");
	OBJECT_ASSERT(child, "widget");
	
	dprintf("attach %s to %s\n", OBJECT(child)->name,
	    OBJECT(parent)->name);
	
	wid->win = reg->win;

	if (OBJECT_OPS(wid)->onattach != NULL) {
		OBJECT_OPS(wid)->onattach(reg->win, wid);
	}

	TAILQ_INSERT_TAIL(&reg->widgetsh, wid, widgets);
	
	/* Reposition/resize regions and widgets. */
	window_resize(wid->win);
	wid->win->redraw++;
}

/*
 * Detach a widget from this region.
 * Window must be locked.
 */
void
region_detach(void *parent, void *child)
{
	struct region *reg = parent;
	struct widget *wid = child;

	OBJECT_ASSERT(parent, "window-region");
	OBJECT_ASSERT(child, "widget");

	dprintf("detach %s from %s\n", OBJECT(child)->name,
	    OBJECT(parent)->name);

	if (OBJECT_OPS(wid)->ondetach != NULL) {
		OBJECT_OPS(wid)->ondetach(wid->win, wid);
	}

	TAILQ_REMOVE(&reg->widgetsh, wid, widgets);
	wid->win->redraw++;
}

