/*	$Csoft: tileset.c,v 1.1 2004/11/21 11:15:44 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/widget/window.h>

#include "tileset.h"

const struct version tileset_ver = {
	"agar tileset",
	0, 0
};

const struct object_ops tileset_ops = {
	tileset_init,
	NULL,			/* reinit */
	NULL,			/* destroy */
	tileset_load,
	tileset_save,
	tileset_edit
};

void
tileset_init(void *obj, const char *name)
{
	struct tileset *ts = obj;

	object_init(ts, "tileset", name, &tileset_ops);
	pthread_mutex_init(&ts->lock, NULL);
	gfx_alloc_pvt(ts, "tiles");
}

void
tileset_destroy(void *obj)
{
	/* nothing yet */
}

int
tileset_load(void *obj, struct netbuf *buf)
{
	struct tileset *ts = obj;

	if (version_read(buf, &tileset_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&ts->lock);
	pthread_mutex_unlock(&ts->lock);
	return (0);
}

int
tileset_save(void *obj, struct netbuf *buf)
{
	struct tileset *ts = obj;

	version_write(buf, &tileset_ver);

	pthread_mutex_lock(&ts->lock);
	pthread_mutex_unlock(&ts->lock);
	return (0);
}

struct window *
tileset_edit(void *p)
{
	struct tileset *ts = p;
	struct window *win;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Tile set: %s"), OBJECT(ts)->name);
	return (win);
}
