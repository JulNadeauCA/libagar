/*	$Csoft: widget.c,v 1.10 2002/04/30 01:11:34 vedge Exp $	*/

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
#include <errno.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "window.h"
#include "widget.h"

extern TAILQ_HEAD(, widget) uwidgetsh;		/* window.c */
extern pthread_mutex_t uwidgets_lock;	/* window.c */

static void	*widget_dispatch_event(void *);

void
widget_init(struct widget *wid, char *name, void *vecp, Sint16 x, Sint16 y,
    Uint16 w, Uint16 h)
{
	static Uint32 widid = 0;
	char *widname;

	/* Prepend parent window's name */
	widname = object_name(name, widid++);
	object_init(&wid->obj, widname, "widget", OBJ_ART, vecp);
	free(widname);

	wid->flags = 0;
	wid->win = NULL;
	wid->x = x;
	wid->y = y;

	/* Geometry may be undefined at this point. */
	wid->w = w;
	wid->h = h;
}

/* The window's widget lock must be held. */
void
widget_link(void *ob, struct window *win)
{
	struct widget *w = (struct widget *)ob;

	w->win = win;
	dprintf("%s widget linked to %s\n", OBJECT(ob)->name,
	    OBJECT(win)->name);

	if (WIDGET_VEC(w)->widget_link != NULL) {
		WIDGET_VEC(w)->widget_link(ob, win);
	}

	TAILQ_INSERT_HEAD(&w->win->widgetsh, w, widgets);

	w->win->redraw++;
}

/* The window's lock must be held. */
void
widget_unlink(void *ob)
{
	struct widget *w = (struct widget *)ob;

	if (WIDGET_VEC(w)->widget_unlink != NULL) {
		WIDGET_VEC(w)->widget_unlink(w);
	}

	/*
	 * Queue the unlink operation, the event loop cannot modify
	 * a list it might be traversing.
	 */
	pthread_mutex_lock(&uwidgets_lock);
	TAILQ_INSERT_HEAD(&uwidgetsh, w, uwidgets);
	pthread_mutex_unlock(&uwidgets_lock);
}

/*
 * Render a widget.
 * Parent window's widget list must be locked.
 */
void
widget_draw(void *p)
{
	struct widget *w = (struct widget *)p;
	
	if (WIDGET_VEC(w)->widget_draw != NULL && !(w->flags & WIDGET_HIDE)) {
		WIDGET_VEC(w)->widget_draw(w);
	}
}

/*
 * Dispatch an event to a widget. The event handler runs in a newly
 * created thread, to avoid blocking in event context.
 * Parent window's widget list must be locked.
 */
void
widget_event(void *p, SDL_Event *ev, int flags)
{
	struct widget *w = (struct widget *)p;

	if (WIDGET_VEC(w)->widget_event != NULL) {
		pthread_t wevent_th;
		struct widget_event *wev;

		/* XXX pool */
		wev = emalloc(sizeof(struct widget_event));
		wev->widget = w;
		wev->ev = *ev;		/* Structure copy */
		wev->flags = flags;

		/*
		 * Widget events are processed in threads spawned from
		 * the event thread. A copy of the SDL_Event is passed.
		 */
		if (pthread_create(&wevent_th, NULL, widget_dispatch_event,
		    wev) != 0) {
			fatal("wevent_th: %s\n", strerror(errno));
		}
	}
}

static void *
widget_dispatch_event(void *arg)
{
	struct widget_event *wev = (struct widget_event *)arg;

	WIDGET_VEC(wev->widget)->widget_event(wev->widget, &wev->ev,
	    wev->flags);

	free(wev);	/* XXX pool */
	return (NULL);
}

