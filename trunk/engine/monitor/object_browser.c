/*	$Csoft: object_browser.c,v 1.28 2003/04/12 01:40:53 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <config/debug.h>
#include <config/floating_point.h>

#ifdef DEBUG

#include <engine/engine.h>
#include <engine/view.h>

#include <string.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/textbox.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapview.h>

#include "monitor.h"

static void	objs_poll(int, union evarg *);
static void	objs_show(int, union evarg *);

static void	props_update(int, union evarg *);
static void	props_remove(int, union evarg *);
static void	props_change(int, union evarg *);

/* Update the event handler list. */
static void
events_poll(int argc, union evarg *argv)
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

/* Display event handler information. */
static void
events_show(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct window *win;
	struct region *reg;
	
	if ((win = window_generic_new(277, 142, "monitor-ob-%s-evh", ob->name))
	    == NULL) {
		return;
	}
	window_set_caption(win, "%s: event handlers", ob->name);

	/* Display the event handlers. */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", events_poll, "%p", ob);
		event_new(tl, "tlist-changed", events_show, "%p", ob);
	}
	window_show(win);
}

/* Update the list of generic properties. */
static void
props_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct prop *prop;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ob->props_lock);

	TAILQ_FOREACH(prop, &ob->props, props) {
		SDL_Surface *su;

		su = SPRITE(&monitor, MONITOR_PROPS_BASE+1+prop->type);
		tlist_insert_item(tl, su, prop->key, prop);
	}

	pthread_mutex_unlock(&ob->props_lock);
	tlist_restore_selections(tl);
}

/* Display the generic properties of an object. */
static void
props_show(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct window *win;
	struct region *reg;
	
	if ((win = window_generic_new(277, 142, "monitor-ob-%s-props",
	    ob->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s: props", ob->name);

	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", props_poll, "%p", ob);

		reg = region_new(win, REGION_VALIGN, 50, 35, 50, 20);
		{
			struct label *lab_name;
			struct textbox *tb_set;
			struct button *bu;

			lab_name = label_new(reg, 100, 30, "");
			tb_set = textbox_new(reg, "Value: ", 0, 100, -1);

			event_new(tl, "tlist-changed",
			    props_update, "%p, %p", lab_name, tb_set);
		
			reg = region_new(win, REGION_HALIGN, 50, 60, 50, 10);
			{
				bu = button_new(reg, "Apply", NULL, 0, 50, -1);
				event_new(bu, "button-pushed",
				    props_change, "%p, %p, %p", tl, tb_set, ob);
			
				bu = button_new(reg, "Remove", NULL, 0, 50, -1);
				event_new(bu, "button-pushed",
				    props_remove, "%p, %p, %p", tl, tb_set, ob);
			}
		}
	}
	window_show(win);
}

/* Display an object's descendants. */
static void
childs_show(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct window *win;
	struct region *reg;
	struct button *showbu;
	
	if ((win = window_generic_new(277, 142, "monitor-ob-%s-childs",
	    ob->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s: descendants", ob->name);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		showbu = button_new(reg, "Show", NULL, 0, 100, -1);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 0, TLIST_POLL);
		event_new(tl, "tlist-poll", objs_poll, "%p", ob);
		event_new(showbu, "button-pushed", objs_show, "%p", tl);
	}
	window_show(win);

}

/* Update an object tree display. */
static void
objs_poll_update(struct tlist *tl, struct object *parent)
{
	struct object *ob;
	char *name;

	pthread_mutex_lock(&parent->lock);

	name = object_name(parent);
	tlist_insert_item(tl, OBJECT_ICON(parent), name, parent);
	free(name);

	TAILQ_FOREACH(ob, &parent->childs, cobjs)
		objs_poll_update(tl, ob);			/* Recurse */

	pthread_mutex_unlock(&parent->lock);
}

/* Update a list of objects attached to a given parent. */
static void
objs_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *parent = argv[1].p;

	tlist_clear_items(tl);
	objs_poll_update(tl, parent);
	tlist_restore_selections(tl);
}

/* Display information about an object. */
static void
objs_show(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct object *ob;
	struct window *win;
	struct region *reg;

	if ((it = tlist_item_selected(tl)) == NULL) {
		text_msg("Error", "No object is selected.");
		return;
	}
	ob = it->p1;

	if ((win = window_generic_new(316, 154, "monitor-ob-%s", ob->name))
	    == NULL) {
		return;
	}
	window_set_caption(win, "%s object", ob->name);
	
	reg = region_new(win, REGION_VALIGN, 0, -1, 80, -1);
	{
		label_new(reg, 100, -1, "Name: %s", ob->name);
		label_new(reg, 100, -1, "Type: %s", ob->type);
		label_polled_new(reg, 100, -1, &ob->lock, "Flags: 0x%x",
		    &ob->flags);
		label_polled_new(reg, 100, -1, &ob->lock, "Gfx: %p", &ob->art);
		label_polled_new(reg, 100, -1, &ob->lock, "Pos: %p", &ob->pos);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		struct button *bu;

		bu = button_new(reg, "Properties", NULL, 0, 33, 0);
		event_new(bu, "button-pushed", props_show, "%p", ob);
		
		bu = button_new(reg, "Child objects", NULL, 0, 33, 0);
		event_new(bu, "button-pushed", childs_show, "%p", ob);
		
		bu = button_new(reg, "Event handlers", NULL, 0, 33, 0);
		event_new(bu, "button-pushed", events_show, "%p", ob);
	}
	
	window_show(win);
}

/* Update a property display. */
static void
props_update(int argc, union evarg *argv)
{
	struct label *lab_name = argv[1].p;
	struct textbox *tb_set = argv[2].p;
	struct tlist_item *it = argv[3].p;
	int selected = argv[4].i;
	struct prop *prop = it->p1;
	
	tb_set->flags &= ~(TEXTBOX_READONLY);

	if (selected) {
		label_printf(lab_name, "%s", prop->key);
		switch (prop->type) {
		case PROP_INT:
			textbox_printf(tb_set, "%d", prop->data.i);
			break;
		case PROP_UINT8:
			textbox_printf(tb_set, "%d", prop->data.u8);
			break;
		case PROP_SINT8:
			textbox_printf(tb_set, "%d", prop->data.s8);
			break;
		case PROP_UINT16:
			textbox_printf(tb_set, "%d", prop->data.u16);
			break;
		case PROP_SINT16:
			textbox_printf(tb_set, "%d", prop->data.s16);
			break;
		case PROP_UINT32:
			textbox_printf(tb_set, "%d", prop->data.u32);
			break;
		case PROP_SINT32:
			textbox_printf(tb_set, "%d", prop->data.s32);
			break;
#ifdef FLOATING_POINT
		case PROP_FLOAT:
			textbox_printf(tb_set, "%f", prop->data.f);
			break;
		case PROP_DOUBLE:
			textbox_printf(tb_set, "%f", prop->data.d);
			break;
#endif
		case PROP_STRING:
			textbox_printf(tb_set, "%s", prop->data.s);
			break;
		case PROP_POINTER:
			textbox_printf(tb_set, "%p", prop->data.p);
			tb_set->flags |= TEXTBOX_READONLY;
			break;
		case PROP_BOOL:
			textbox_printf(tb_set, "%s",
			    prop->data.i ? "true" : "false");
			break;
		default:
			break;
		};
	} else {
		label_printf(lab_name, ".");
		textbox_printf(tb_set, "");
	}
}

/* Remove a generic property. */
static void
props_remove(int argc, union evarg *argv)
{
	struct tlist *propstl = argv[1].p;
	struct object *ob = argv[3].p;
	struct tlist_item *it;
	struct prop *prop;

	it = tlist_item_selected(propstl);
	if (it == NULL) {
		text_msg("Error", "No property is selected.");
		return;
	}
	prop = it->p1;

	pthread_mutex_lock(&ob->props_lock);
	TAILQ_REMOVE(&ob->props, prop, props);
	prop_destroy(prop);
	free(prop);
	pthread_mutex_unlock(&ob->props_lock);
}

/*
 * Alter the value of a property.
 * XXX move the translation to prop.c.
 */
static void
props_change(int argc, union evarg *argv)
{
	struct tlist *propstl = argv[1].p;
	struct textbox *tb_set = argv[2].p;
	struct object *ob = argv[3].p;
	struct tlist_item *it;
	struct prop *prop;

	it = tlist_item_selected(propstl);
	if (it == NULL) {
		text_msg("Error", "No property is selected.");
		return;
	}
	prop = it->p1;

	pthread_mutex_lock(&tb_set->text.lock);

	switch (prop->type) {
	case PROP_INT:
		prop_set_int(ob, prop->key, atoi(tb_set->text.s));
		break;
	case PROP_UINT8:
		prop_set_uint8(ob, prop->key, (Uint8)atoi(tb_set->text.s));
		break;
	case PROP_SINT8:
		prop_set_sint8(ob, prop->key, (Sint8)atoi(tb_set->text.s));
		break;
	case PROP_UINT16:
		prop_set_uint16(ob, prop->key, (Uint16)atoi(tb_set->text.s));
		break;
	case PROP_SINT16:
		prop_set_sint16(ob, prop->key, (Sint16)atoi(tb_set->text.s));
		break;
	case PROP_UINT32:
		prop_set_uint32(ob, prop->key, (Uint32)atoi(tb_set->text.s));
		break;
	case PROP_SINT32:
		prop_set_sint32(ob, prop->key, (Sint32)atoi(tb_set->text.s));
		break;
#ifdef FLOATING_POINT
	case PROP_FLOAT:
		prop_set_float(ob, prop->key, (float)atof(tb_set->text.s));
		break;
	case PROP_DOUBLE:
		prop_set_double(ob, prop->key, atof(tb_set->text.s));
		break;
#endif
	case PROP_STRING:
		prop_set_string(ob, prop->key, tb_set->text.s);
		break;
	case PROP_BOOL:
		if (strcasecmp(tb_set->text.s, "true") == 0) {
			prop_set_bool(ob, prop->key, 1);
		} else if (strcasecmp(tb_set->text.s, "false") == 0) {
			prop_set_bool(ob, prop->key, 0);
		} else {
			text_msg("Error", "Bad boolean string.");
		}
		break;
	default:
		text_msg("Error", "Read-only property type.");
		break;
	}
	
	pthread_mutex_unlock(&tb_set->text.lock);
}

struct window *
object_browser_window(void)
{
	struct window *win;
	struct region *reg;

	if ((win = window_generic_new(251, 259, "monitor")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, "Objects");
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-changed", objs_show, "%p", tl);
		event_new(tl, "tlist-poll", objs_poll, "%p", world);
	}
	return (win);
}

#endif	/* DEBUG */
