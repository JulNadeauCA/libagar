/*	$Csoft: gobject.c,v 1.1 2005/02/08 15:57:18 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "gobject.h"

const struct version gobject_ver = {
	"agar geometric object",
	0, 0
};

void
gobject_init(void *obj, const char *type, const char *name,
    const struct gobject_ops *ops)
{
	struct gobject *gobj = obj;

	object_init(gobj, type, name, ops);
	pthread_mutex_init(&gobj->lock, NULL);
}

void
gobject_reinit(void *obj)
{
	struct object *gobj = obj;

	pthread_mutex_lock(&gobj->lock);
	pthread_mutex_unlock(&gobj->lock);
}

void
gobject_destroy(void *obj)
{
	/* nothing yet */
}

int
gobject_load(void *obj, struct netbuf *buf)
{
	struct gobject *gobj = obj;

	if (version_read(buf, &gobject_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&gobj->lock);
	pthread_mutex_unlock(&gobj->lock);
	return (0);
}

int
gobject_save(void *obj, struct netbuf *buf)
{
	struct gobject *gobj = obj;

	version_write(buf, &gobject_ver);

	pthread_mutex_lock(&gobj->lock);
	pthread_mutex_unlock(&gobj->lock);
	return (0);
}

struct window *
gobject_edit(void *obj)
{
	struct gobject *gobj = obj;
	struct window *win;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Geometric object: %s"), OBJECT(gobj)->name);

	return (win);
}
