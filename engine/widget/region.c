/*	$Csoft: region.c,v 1.30 2003/03/25 13:48:08 vedge Exp $	*/

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

#include <engine/compat/snprintf.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

static const struct object_ops region_ops = {
	region_destroy,
	NULL,		/* load */
	NULL		/* save */
};

pthread_mutex_t region_lock = PTHREAD_MUTEX_INITIALIZER;
Uint32		region_count = 0;

struct region *
region_new(void *parent, int flags, int rx, int ry, int rw, int rh)
{
	struct window *win = parent;
	struct region *reg;

	OBJECT_ASSERT(parent, "window");

	reg = Malloc(sizeof(struct region));
	region_init(reg, flags, rx, ry, rw, rh);
	window_attach(win, reg);
	return (reg);
}

void
region_init(struct region *reg, int flags, int rx, int ry, int rw, int rh)
{
	char name[OBJECT_NAME_MAX];

	pthread_mutex_lock(&region_lock);
	region_count++;
	pthread_mutex_unlock(&region_lock);

	snprintf(name, sizeof(name), "region%u", region_count);
	object_init(&reg->obj, "window-region", name, NULL, 0, &region_ops);

	reg->flags = (flags != 0) ? flags : REGION_HALIGN;
	reg->rx = rx;
	reg->ry = ry;
	reg->rw = rw;
	reg->rh = rh;
	reg->x = 0;
	reg->y = 0;
	reg->w = 0;
	reg->h = 0;
	reg->xspacing = 4;
	reg->yspacing = 4;
	reg->win = NULL;
	TAILQ_INIT(&reg->widgets);
	reg->nwidgets = 0;
}

void
region_destroy(void *p)
{
	struct region *reg = p;
	struct widget *wid, *nextwid;

	for (wid = TAILQ_FIRST(&reg->widgets);
	     wid != TAILQ_END(&reg->widgets);
	     wid = nextwid) {
		nextwid = TAILQ_NEXT(wid, widgets);
		event_post(wid, "detached", p);		/* Notify */
		object_destroy(wid);			/* Free */
		free(wid);
	}
	TAILQ_INIT(&reg->widgets);
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
	TAILQ_INSERT_TAIL(&reg->widgets, wid, widgets);		/* Attach */
	reg->nwidgets++;
	pthread_mutex_unlock(&reg->win->lock);
	
	OBJECT(child)->state = OBJECT_CONSISTENT;
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
	TAILQ_REMOVE(&reg->widgets, wid, widgets);		/* Detach */
	if (--reg->nwidgets < 0) {
		fatal("nwidgets < 0");
	}
	pthread_mutex_unlock(&reg->win->lock);
	
	event_post(wid, "detached", "%p", parent);		/* Notify */

	OBJECT(wid)->state = OBJECT_DETACHED;
	object_destroy(wid);
	free(wid);
}

/* Change the spacing between widgets. */
void
region_set_spacing(struct region *reg, Uint8 xsp, Uint8 ysp)
{
	pthread_mutex_lock(&reg->win->lock);
	reg->xspacing = xsp;
	reg->yspacing = ysp;
	pthread_mutex_unlock(&reg->win->lock);
}
