/*	$Csoft: widget.c,v 1.5 2002/04/24 14:08:54 vedge Exp $	*/

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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "window.h"
#include "widget.h"

static const struct obvec widget_vec = {
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	widget_link,
	widget_unlink
};

void
widget_init(struct widget *wid, char *name, void *vecp,
    struct window *win, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	char *widname;

	/* Prepend parent window's name */
	widname = emalloc(strlen(name) + strlen(OBJECT(win)->name) + 2);
	strcpy(widname, OBJECT(win)->name);
	strcat(widname, ".");
	strcat(widname, name);

	object_init(&wid->obj, widname, "widget", OBJ_ART, vecp);

	wid->flags = 0;
	wid->win = win;
	wid->x = x;
	wid->y = y;

	/* Geometry may be undefined at this point. */
	wid->w = w;
	wid->h = h;
}

int
widget_link(void *ob)
{
	struct widget *w = (struct widget *)ob;

	pthread_mutex_lock(&w->win->widgetslock);
	TAILQ_INSERT_HEAD(&w->win->widgetsh, w, widgets);
	w->win->nwidgets++;
	pthread_mutex_unlock(&w->win->widgetslock);

	w->win->redraw++;
	return (0);
}

/* The window's widget lock must be held. */
int
widget_unlink(void *ob)
{
	struct widget *w = (struct widget *)ob;
	struct window *win = w->win;

	TAILQ_REMOVE(&w->win->widgetsh, w, widgets);
	w->win->nwidgets--;

	win->redraw++;
	return (0);
}

/*
 * Render a widget.
 * Parent window's widget list must be locked.
 */
void
widget_draw(void *p)
{
	struct widget *w = (struct widget *)p;
	
	if (WIDGET_VEC(w)->draw != NULL && !(w->flags & WIDGET_HIDE)) {
		WIDGET_VEC(w)->draw(w);
	}
}

/*
 * Dispatch an event to a widget.
 * Parent window's widget list must be locked.
 */
void
widget_event(void *p, SDL_Event *ev, Uint32 flags)
{
	struct widget *w = (struct widget *)p;

	if (WIDGET_VEC(w)->event != NULL) {
		dprintf("event to %s\n", OBJECT(w)->name);
		WIDGET_VEC(w)->event(w, ev, flags);
	}
}


