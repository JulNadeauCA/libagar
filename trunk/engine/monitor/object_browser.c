/*	$Csoft: object_browser.c,v 1.3 2002/11/14 07:59:28 vedge Exp $	*/

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

/* An object was attached to the world. */
void
object_browser_attached_object(int argc, union evarg *argv)
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

/* An object was detached from the world. */
void
object_browser_detached_object(int argc, union evarg *argv)
{
	fatal("remove\n");
}

static void
trigger_event(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct event *evh = argv[2].p;

	/* Fake this event with no arguments. */
	event_post(ob, evh->name, NULL);
}

static void
unregister_event(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct tlist *event_list = argv[1].p;
	struct object *ob = argv[2].p;
	struct event *evh = argv[3].p;
	struct tlist_item *event_item;

	/* Unregister the event handler. */
	pthread_mutex_lock(&ob->events_lock);
	TAILQ_REMOVE(&ob->events, evh, events);
	pthread_mutex_unlock(&ob->events_lock);

	/* Destroy the event handler window. */
	view_detach(WIDGET(bu)->win);
}

static void
event_list_selected(int argc, union evarg *argv)
{
	struct tlist *event_list = argv[0].p;
	struct object *ob = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct event *evh = it->p1;
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *bu;
	int i;

	win = window_generic_new(215, 140, "%s handler", evh->name);
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 70);
	lab = label_new(reg, 100, 0, "Identifier: \"%s\"", evh->name);
	lab = label_new(reg, 100, 0, "Flags: 0x%x", evh->flags);
	lab = label_new(reg, 100, 0, "Handler: %p", evh->handler);

	reg = region_new(win, REGION_HALIGN, 0, 70, 100, 30);
	bu = button_new(reg, "Trigger", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", trigger_event, "%p, %p", ob, evh);

	bu = button_new(reg, "Unregister", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", unregister_event, "%p, %p, %p",
	    event_list, ob, evh);

	window_show(win);
}

/* Update the event list. */
static void
event_list_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct event *evh;

	tlist_clear_items(tl);

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_FOREACH(evh, &ob->events, events) {
		tlist_insert_item(tl, NULL, evh->name, evh);
	}
	pthread_mutex_unlock(&ob->events_lock);
}

static void
objlist_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object_browser *obr = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct object *ob = it->p1;
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct tlist *etl;
	struct event *evh;

	win = window_new(NULL, "Object structure", WINDOW_CENTER, -1, -1,
	    394, 307, 251, 240);
	
	/* Show the object's generic properties. */
	reg = region_new(win, REGION_VALIGN, 0, 0, 60, 40);
	lab = label_new(reg, 100, 0, "Name: %s", ob->name);
	lab = label_new(reg, 100, 0, "Type: %s", ob->type);
	lab = label_new(reg, 100, 0, "Flags: 0x%x", ob->flags);
	lab = label_new(reg, 100, 0, "State: %s",
	    ob->state == OBJECT_EMBRYONIC ? "EMBRYONIC" :
	    ob->state == OBJECT_CONSISTENT ? "CONSISTENT" :
	    ob->state == OBJECT_ZOMBIE ? "ZOMBIE" : "???");

	/* Display the first sprite, if any. */
	if (ob->art != NULL && ob->art->nsprites > 0) {
		reg = region_new(win, REGION_VALIGN, 60, 0, 40, 40);
		bitmap_new(reg, SPRITE(ob, 0), 100, 100);
	}

	/* Display a polling list of event handlers. */
	reg = region_new(win, REGION_VALIGN, 60, 40, 40, 60);
	etl = tlist_new(reg, 100, 100, TLIST_POLL);
	event_new(etl, "tlist-poll", event_list_poll, "%p", ob);
	event_new(etl, "tlist-changed", event_list_selected, "%p", ob);

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
	event_new(obr->objlist, "tlist-changed", objlist_selected, "%p", obr);

	return (win);
}

struct object_browser *
object_browser_new(struct monitor *mon, int flags)
{
	struct object_browser *object_browser;

	object_browser = emalloc(sizeof(struct object_browser));
	object_browser_init(object_browser, mon, flags);

	return (object_browser);
}

void
object_browser_init(struct object_browser *object_browser, struct monitor *mon,
    int flags)
{
	monitor_tool_init(&object_browser->tool, "object_browser", mon,
	    &object_browser_ops);

	object_browser->flags = flags;
}

#endif	/* DEBUG */
