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

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "widget.h"
#include "window.h"

static struct obvec window_vec = {
	window_destroy,
	window_load,
	window_save,
	window_link,
	window_unlink
};

static TAILQ_HEAD(, window) windowsh = TAILQ_HEAD_INITIALIZER(windowsh);
static pthread_mutex_t windowslock = PTHREAD_MUTEX_INITIALIZER;

struct window *
window_create(struct viewport *view, char *name, Uint32 flags, SDL_Rect rect,
    Uint32 *bgcolor, Uint32 *fgcolor)
{
	struct window *w;

	w = (struct window *)emalloc(sizeof(struct window));
	object_init(&w->obj, name, 0, &window_vec);

	w->caption = strdup(name);

	w->view = view;
	w->flags = flags;
	w->rect = rect;
	w->bgcolor = bgcolor;
	w->fgcolor = fgcolor;

	w->vmask.x = (rect.x / view->map->tilew) - view->mapxoffs;
	w->vmask.y = (rect.y / view->map->tileh) - view->mapyoffs;
	w->vmask.w = (rect.w / view->map->tilew);
	w->vmask.h = (rect.h / view->map->tilew);

	view_maskfill(view, &w->vmask, 1);

	return (w);
}

int
window_load(void *p, int fd)
{
	struct window *w = (struct window *)p;

	if (version_read(fd, "agar window", 1, 0) != 0) {
		return (-1);
	}
	
	w->flags = fobj_read_uint32(fd);
	w->caption = fobj_read_string(fd);
	*w->bgcolor = fobj_read_uint32(fd);
	*w->fgcolor = fobj_read_uint32(fd);
	w->rect = fobj_read_rect(fd);

	return (0);
}

int
window_save(void *p, int fd)
{
	struct window *w = (struct window *)p;

	version_write(fd, "agar window", 1, 0);

	fobj_write_uint32(fd, w->flags);
	fobj_write_string(fd, w->caption);
	fobj_write_uint32(fd, *w->bgcolor);
	fobj_write_uint32(fd, *w->fgcolor);
	fobj_write_rect(fd, w->rect);

	return (0);
}

int
window_link(void *ob)
{
	struct window *w = (struct window *)ob;

	pthread_mutex_lock(&windowslock);
	TAILQ_INSERT_HEAD(&windowsh, w, windows);
	pthread_mutex_unlock(&windowslock);

	return (0);
}

int
window_unlink(void *ob)
{
	struct window *w = (struct window *)ob;
	
	pthread_mutex_lock(&windowslock);
	TAILQ_REMOVE(&windowsh, w, windows);
	pthread_mutex_unlock(&windowslock);

	return (0);
}

int
window_destroy(void *p)
{
	free(w->caption);

	return (0);
}

