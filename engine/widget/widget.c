/*	$Csoft: widget.c,v 1.24 2002/09/07 04:36:05 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <engine/engine.h>

#include "widget.h"
#include "window.h"

void
widget_init(struct widget *wid, char *name, char *style, const void *wops,
    int rw, int rh)
{
	static Uint32 widid = 0;
	static pthread_mutex_t widid_lock = PTHREAD_MUTEX_INITIALIZER;
	char *widname;

	pthread_mutex_lock(&widid_lock);
	widid++;
	pthread_mutex_unlock(&widid_lock);

	widname = object_name(name, widid);
	object_init(&wid->obj, "widget", widname, style,
	    OBJECT_ART|OBJECT_KEEP_MEDIA, wops);

	free(widname);

	wid->type = strdup(name);
	wid->flags = 0;
	wid->win = NULL;
	wid->x = 0;
	wid->y = 0;
	wid->rw = rw;
	wid->rh = rh;
	wid->w = 0;
	wid->h = 0;
	wid->ncolors = 0;
	SLIST_INIT(&wid->colors);

	wid->surface.redraw = 0;
	wid->surface.source = NULL;
	pthread_mutex_init(&wid->surface.lock, NULL);
}

void
widget_map_color(void *p, int ind, char *name, Uint8 r, Uint8 g, Uint8 b)
{
	struct widget *wid = p;
	struct widget_color *col;
	
	OBJECT_ASSERT(wid, "widget");
	
	if (ind > WIDGET_MAXCOLORS)
		fatal("%d colors > %d\n", ind, WIDGET_MAXCOLORS);
	if (ind > wid->ncolors)
		wid->ncolors++;

	wid->color[ind] = SDL_MapRGB(view->v->format, r, g, b);

	col = emalloc(sizeof(struct widget_color));
	col->name = strdup(name);
	col->ind = ind;
	SLIST_INSERT_HEAD(&wid->colors, col, colors);
}

void
widget_destroy(void *p)
{
	struct widget *wid = p;

	if (wid->surface.source != NULL) {
		SDL_FreeSurface(wid->surface.source);
	}
	pthread_mutex_destroy(&wid->surface.lock);
}

void
widget_attach(void *parent, void *child)
{
	struct widget *p = parent;
	struct widget *c = child;

	OBJECT_ASSERT(parent, "widget");
	OBJECT_ASSERT(child, "widget");

	dprintf("attach %p to %p\n", OBJECT(c)->name, OBJECT(p)->name);
}

void
widget_detach(void *parent, void *child)
{
	struct widget *p = parent;
	struct widget *c = child;

	OBJECT_ASSERT(p, "widget");
	OBJECT_ASSERT(child, "widget");

	dprintf("detach %p from %p\n", OBJECT(c)->name, OBJECT(p)->name);
}
