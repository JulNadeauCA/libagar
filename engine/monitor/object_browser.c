/*	$Csoft: object_browser.c,v 1.2 2002/11/14 05:59:02 vedge Exp $	*/

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

#include <engine/mcconfig.h>

#ifdef DEBUG

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"
#include "object_browser.h"

static const struct monitor_tool_ops object_browser_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	object_browser_window
};

struct object_browser *
object_browser_new(struct monitor *mon, int flags)
{
	struct object_browser *object_browser;

	object_browser = emalloc(sizeof(struct object_browser));
	object_browser_init(object_browser, mon, flags);

	return (object_browser);
}

void
object_browser_attached(int argc, union evarg *argv)
{
	struct object_browser *obr = argv[0].p;
	struct object *ob = argv[1].p;
	SDL_Surface *icon = NULL;

	if (ob->flags & OBJECT_ART && ob->art != NULL &&
	    ob->art->nsprites > 0) {
		icon = SPRITE(ob, 0);
	}
	tlist_insert_item(obr->objlist, icon, ob->name, ob);
}

void
object_browser_detached(int argc, union evarg *argv)
{
	fatal("remove\n");
}

void
object_browser_init(struct object_browser *object_browser, struct monitor *mon,
    int flags)
{
	monitor_tool_init(&object_browser->tool, "object_browser", mon,
	    &object_browser_ops);
	event_new(object_browser, "world-attached-object",
	    object_browser_attached, NULL);
	event_new(object_browser, "world-detached-object",
	    object_browser_detached, NULL);

	object_browser->flags = flags;
}

static void
trigger_event(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct event *eev = argv[2].p;

	event_post(ob, eev->name, NULL);
}

static void
unregister_event(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct object *ob = argv[1].p;
	struct event *eev = argv[2].p;

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_REMOVE(&ob->events, eev, events);
	pthread_mutex_unlock(&ob->events_lock);

	view_detach(WIDGET(bu)->win);
}

static void
selected_event(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct event *eev = it->p1;
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *bu;
	int i;

	win = window_new(NULL, "Event handler", WINDOW_CENTER, -1, -1,
	    215, 140, 215, 140);
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 70);
	lab = label_new(reg, 100, 0, "Identifier: \"%s\"", eev->name);
	lab = label_new(reg, 100, 0, "Flags: 0x%x", eev->flags);
	lab = label_new(reg, 100, 0, "Handler: %p", eev->handler);

	reg = region_new(win, REGION_HALIGN, 0, 70, 100, 30);
	bu = button_new(reg, "Trigger", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", trigger_event, "%p, %p", ob, eev);

	bu = button_new(reg, "Unregister", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", unregister_event, "%p, %p", ob, eev);

	window_show(win);
}

static void
selected_object(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it = argv[1].p;
	struct object *ob = it->p1;
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct tlist *etl;
	struct event *eev;

	win = window_new(NULL, "Object structure", WINDOW_CENTER, -1, -1,
	    394, 307, 251, 240);
	reg = region_new(win, REGION_VALIGN, 0, 0, 60, 40);
	lab = label_new(reg, 100, 0, "Name: %s", ob->name);
	lab = label_new(reg, 100, 0, "Type: %s", ob->type);
	lab = label_new(reg, 100, 0, "Flags: 0x%x", ob->flags);
	lab = label_new(reg, 100, 0, "State: %s",
	    ob->state == OBJECT_EMBRYONIC ? "EMBRYONIC" :
	    ob->state == OBJECT_CONSISTENT ? "CONSISTENT" :
	    ob->state == OBJECT_ZOMBIE ? "ZOMBIE" : "???");

	if (ob->art != NULL && ob->art->nsprites > 0) {
		reg = region_new(win, REGION_VALIGN, 60, 0, 40, 40);
		bitmap_new(reg, SPRITE(ob, 0), 100, 100);
	}

	reg = region_new(win, REGION_VALIGN, 60, 40, 40, 60);
	etl = tlist_new(reg, 100, 100, 0);
	event_new(etl, "tlist-changed", selected_event, "%p", ob);

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_FOREACH(eev, &ob->events, events) {
		tlist_insert_item(etl, NULL, eev->name, eev);
	}
	pthread_mutex_unlock(&ob->events_lock);
	window_show(win);
}

struct window *
object_browser_window(void *p)
{
	struct object_browser *obr = p;
	struct window *win;
	struct region *reg;
	struct textbox *obj_tbox, *offs_tbox;
	struct button *button;
	struct bitmap *bmp;

	win = window_new("monitor-media-browser", "Object browser",
	    WINDOW_SOLID|WINDOW_CENTER,
	    0, 0, 184, 100, 184, 100);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	obr->objlist = tlist_new(reg, 100, 100, 0);
	event_new(obr->objlist, "tlist-changed", selected_object, NULL);

	return (win);
}

#endif	/* DEBUG */
