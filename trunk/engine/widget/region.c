/*	$Csoft: region.c,v 1.20 2002/11/15 00:50:38 vedge Exp $	*/

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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <engine/engine.h>

#include "text.h"
#include "widget.h"
#include "window.h"

static const struct object_ops region_ops = {
	region_destroy,
	NULL,		/* load */
	NULL		/* save */
};

struct region *
region_new(void *parent, int flags, int rx, int ry, int rw, int rh)
{
	struct window *win = parent;
	struct region *reg;

	OBJECT_ASSERT(parent, "window");

	reg = emalloc(sizeof(struct region));
	region_init(reg, flags, rx, ry, rw, rh);
	window_attach(win, reg);
	return (reg);
}

void
region_init(struct region *reg, int flags, int rx, int ry, int rw, int rh)
{
	static int curreg = 0;
	char *name;

	name = object_name("window-region", curreg++);
	object_init(&reg->obj, "window-region", name, NULL, 0, &region_ops);
	free(name);

	reg->flags = flags;

	reg->rx = rx;
	reg->ry = ry;
	reg->rw = rw;
	reg->rh = rh;
	reg->x = 0;
	reg->y = 0;
	reg->w = 0;
	reg->h = 0;
	reg->spacing = 5;
	reg->win = NULL;
	TAILQ_INIT(&reg->widgetsh);
}

void
region_destroy(void *p)
{
	struct region *reg = p;
	struct widget *wid, *nextwid;

	for (wid = TAILQ_FIRST(&reg->widgetsh);
	     wid != TAILQ_END(&reg->widgetsh);
	     wid = nextwid) {
		nextwid = TAILQ_NEXT(wid, widgets);
		event_post(wid, "detached", p);		/* Notify */
		object_destroy(wid);			/* Free */
	}
	TAILQ_INIT(&reg->widgetsh);
}

/* Attach a widget to this region. */
void
region_attach(void *parent, void *child)
{
	struct region *reg = parent;
	struct widget *wid = child;

	OBJECT_ASSERT(parent, "window-region");
	OBJECT_ASSERT(child, "widget");
	
	wid->win = reg->win;
	wid->reg = reg;

	pthread_mutex_lock(&reg->win->lock);
	TAILQ_INSERT_TAIL(&reg->widgetsh, wid, widgets);	/* Attach */
	pthread_mutex_unlock(&reg->win->lock);
	
	event_post(child, "attached", "%p", parent);		/* Notify */
}

/* Detach a widget from this region. */
void
region_detach(void *parent, void *child)
{
	struct region *reg = parent;
	struct widget *wid = child;

	OBJECT_ASSERT(parent, "window-region");
	OBJECT_ASSERT(child, "widget");

	pthread_mutex_lock(&reg->win->lock);
	TAILQ_REMOVE(&reg->widgetsh, wid, widgets);		/* Detach */
	pthread_mutex_unlock(&reg->win->lock);
	
	event_post(child, "detached", parent);			/* Notify */

	object_destroy(wid);
}

