/*	$Csoft: object_browser.c,v 1.9 2002/11/22 05:41:43 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/compat/asprintf.h>

#ifdef DEBUG

#include <engine/world.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static void
tl_events_trigger(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct event *evh = argv[2].p;

	/* Fake this event with no arguments. */
	event_post(ob, evh->name, NULL);
}

static void
tl_events_unregister(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct tlist *tl_events = argv[1].p;
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
tl_events_selected(int argc, union evarg *argv)
{
	struct tlist *tl_events = argv[0].p;
	struct object *ob = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct event *evh = it->p1;
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *bu;
	int i;

	win = window_generic_new(215, 140,
	    "monitor-object-browser-%s-evh-%s", ob->name, evh->name);
	if (win == NULL) {
		return;		/* Exists */
	}
	window_set_caption(win, "%s handler", evh->name);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 70);
	lab = label_new(reg, 100, 0, "Identifier: \"%s\"", evh->name);
	lab = label_polled_new(reg, 100, 0, &ob->events_lock,
	    "Flags: 0x%x", &evh->flags);
	lab = label_polled_new(reg, 100, 0, &ob->events_lock,
	    "Handler: %p", &evh->handler);

	reg = region_new(win, REGION_HALIGN, 0, 70, 100, 30);
	bu = button_new(reg, "Trigger", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", tl_events_trigger, "%p, %p", ob, evh);

	bu = button_new(reg, "Unregister", NULL, 0, 50, 100);
	event_new(bu, "button-pushed", tl_events_unregister, "%p, %p, %p",
	    tl_events, ob, evh);

	window_show(win);
}

/* Update the event handler list. */
static void
tl_events_poll(int argc, union evarg *argv)
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

	tlist_restore_selections(tl);
}

/* Update the property list. */
static void
tl_props_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct prop *prop;

	tlist_clear_items(tl);

	pthread_mutex_lock(&ob->props_lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		char *s;

		switch (prop->type) {
		case PROP_INT:
			asprintf(&s, "%s = %d", prop->key, prop->data.i);
			break;
		case PROP_UINT8:
			asprintf(&s, "%s = %u", prop->key, prop->data.u8);
			break;
		case PROP_SINT8:
			asprintf(&s, "%s = %d", prop->key, prop->data.s8);
			break;
		case PROP_UINT16:
			asprintf(&s, "%s = %u", prop->key, prop->data.u16);
			break;
		case PROP_SINT16:
			asprintf(&s, "%s = %d", prop->key, prop->data.s16);
			break;
		case PROP_UINT32:
			asprintf(&s, "%s = %u", prop->key, prop->data.u32);
			break;
		case PROP_SINT32:
			asprintf(&s, "%s = %d", prop->key, prop->data.s32);
			break;
		case PROP_STRING:
			asprintf(&s, "%s = \"%s\"", prop->key, prop->data.s);
			break;
		case PROP_POINTER:
			asprintf(&s, "%s = %p", prop->key, prop->data.p);
			break;
		case PROP_BOOL:
			asprintf(&s, "%s = %s", prop->key,
			    prop->data.i ? "true" : "false");
			break;
		default:
			asprintf(&s, "%s = ???", prop->key);
			break;
		}
		tlist_insert_item(tl, NULL, s, prop);
		free(s);
	}
	pthread_mutex_unlock(&ob->props_lock);

	tlist_restore_selections(tl);
}

/* Update the object list. */
static void
tl_objs_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob;
	
	tlist_clear_items(tl);

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		SDL_Surface *icon = NULL;

		if (ob->art != NULL && ob->art->nsprites > 0) {
			icon = SPRITE(ob, 0);
		}
		tlist_insert_item(tl, icon, ob->name, ob);
	}
	pthread_mutex_unlock(&world->lock);

	tlist_restore_selections(tl);
}

/* Show information about an object. */
static void
tl_objs_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it = argv[1].p;
	struct object *ob = it->p1;
	struct window *win;
	struct region *reg;
	struct event *evh;

	win = window_generic_new(296, 251,
	    "monitor-object-browser-obj-%s", ob->name);
	if (win == NULL) {
		return;		/* Exists */
	}
	window_set_caption(win, "%s object (%s)", ob->name, ob->type);

	/* Show the object's generic properties. */
	reg = region_new(win, REGION_VALIGN, 0, 0, 60, 40);
	{
		label_new(reg, 100, 0, "Name: %s", ob->name);
		label_new(reg, 100, 0, "Type: %s", ob->type);
		label_new(reg, 100, 0, "Flags: 0x%x", ob->flags);
		label_polled_new(reg, 100, 0, NULL,
		    "State: %d", &ob->state);
		label_polled_new(reg, 100, 0, &ob->pos_lock,
		    "Position: %p", &ob->pos);
	}

	/* Display the first sprite, if any. */
	if (ob->art != NULL && ob->art->nsprites > 0) {
		reg = region_new(win, REGION_VALIGN, 60, 0, 40, 40);
		bitmap_new(reg, SPRITE(ob, 0), 100, 100);
	}
	
	reg = region_new(win, 0, 0, 40, 100, 60);
	{
		struct tlist *tl;

		/* Generic properties. */
		tl = tlist_new(reg, 60, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_props_poll, "%p", ob);
#if 0
		event_new(tl, "tlist-changed", tl_props_selected, "%p", ob);
#endif
	
		/* Events handlers */
		tl = tlist_new(reg, 40, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_events_poll, "%p", ob);
		event_new(tl, "tlist-changed", tl_events_selected, "%p", ob);
	}

	window_show(win);
}

struct window *
object_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct tlist *tl_objs;

	if ((win = window_generic_new(184, 100, "monitor-object-browser"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_caption(win, "Object browser");

	reg = region_new(win, 0, 0, 0, 100, 100);
	tl_objs = tlist_new(reg, 100, 100, TLIST_POLL);
	event_new(tl_objs, "tlist-changed", tl_objs_selected, NULL);
	event_new(tl_objs, "tlist-poll", tl_objs_poll, NULL);

	return (win);
}

#endif	/* DEBUG */
