/*	$Csoft: space.c,v 1.1 2004/11/21 11:15:44 vedge Exp $	*/

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

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "space.h"

const struct version space_ver = {
	"agar space",
	0, 0
};

void
space_init(void *obj, const char *type, const char *name, const void *ops)
{
	struct space *sp = obj;

	object_init(sp, type, name, ops);
	pthread_mutex_init(&sp->lock, NULL);
}

void
space_reinit(void *obj)
{
}

void
space_destroy(void *obj)
{
	/* nothing yet */
}

int
space_load(void *obj, struct netbuf *buf)
{
	struct space *sp = obj;

	if (version_read(buf, &space_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&sp->lock);
	pthread_mutex_unlock(&sp->lock);
	return (0);
}

int
space_save(void *obj, struct netbuf *buf)
{
	struct space *sp = obj;

	version_write(buf, &space_ver);

	pthread_mutex_lock(&sp->lock);
	pthread_mutex_unlock(&sp->lock);
	return (0);
}

struct window *
space_edit(void *p)
{
	struct space *sp = p;
	struct window *win;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Space %s"), OBJECT(sp)->name);
	return (win);
}
