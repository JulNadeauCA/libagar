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

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "window.h"
#include "widget.h"

static struct obvec widget_vec = {
	widget_destroy,
	widget_load,
	widget_save,
	widget_link,
	widget_unlink
};

static TAILQ_HEAD(, widget) widgetsh = TAILQ_HEAD_INITIALIZER(widgetsh);
static pthread_mutex_t widgetslock = PTHREAD_MUTEX_INITIALIZER;

struct widget *
widget_create(struct window *win, char *name, Uint32 flags, SDL_Rect rect,
    Uint32 *bgcolor, Uint32 *fgcolor)
{
	struct widget *w;
	struct viewport *view;

	w = (struct widget *)emalloc(sizeof(struct widget));
	object_init(&w->obj, name, 0, &widget_vec);

	w->win = win;
	w->flags = flags;
	w->rect = rect;
	w->bgcolor = bgcolor;
	w->fgcolor = fgcolor;

	view = w->win->view;

	w->vmask.x = (rect.x / view->map->tilew) - view->mapxoffs;
	w->vmask.y = (rect.y / view->map->tileh) - view->mapyoffs;
	w->vmask.w = (rect.w / view->map->tilew);
	w->vmask.h = (rect.h / view->map->tilew);

	view_maskfill(view, &w->vmask, 1);

	return (w);
}

int
widget_load(void *p, int fd)
{
	struct widget *w = (struct widget *)p;

	if (version_read(fd, "agar widget", 1, 0) != 0) {
		return (-1);
	}
	
	w->flags = fobj_read_uint32(fd);
	*w->bgcolor = fobj_read_uint32(fd);
	*w->fgcolor = fobj_read_uint32(fd);
	w->rect = fobj_read_rect(fd);

	return (0);
}

int
widget_save(void *p, int fd)
{
	struct widget *w = (struct widget *)p;

	version_write(fd, "agar widget", 1, 0);

	fobj_write_uint32(fd, w->flags);
	fobj_write_uint32(fd, *w->bgcolor);
	fobj_write_uint32(fd, *w->fgcolor);
	fobj_write_rect(fd, w->rect);

	return (0);
}

int
widget_link(void *ob)
{
	struct widget *w = (struct widget *)ob;

	pthread_mutex_lock(&widgetslock);
	TAILQ_INSERT_HEAD(&widgetsh, w, widgets);
	pthread_mutex_unlock(&widgetslock);

	return (0);
}

int
widget_unlink(void *ob)
{
	struct widget *w = (struct widget *)ob;
	
	pthread_mutex_lock(&widgetslock);
	TAILQ_REMOVE(&widgetsh, w, widgets);
	pthread_mutex_unlock(&widgetslock);

	return (0);
}

int
widget_destroy(void *p)
{
	/* ... */

	return (0);
}

