/*	$Csoft: position.c,v 1.4 2003/10/14 03:15:26 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/input.h>

#include "position.h"

#include <engine/widget/tlist.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/combo.h>
#include <engine/widget/spinbutton.h>

static int	position_cursor(void *, struct mapview *, SDL_Rect *);
static void	position_effect(void *, struct mapview *, struct map *,
		                struct node *);

const struct tool_ops position_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	position_cursor,
	position_effect,
	NULL			/* mouse */
};

static struct tlist_item *
find_objs(struct tlist *tl, struct object *pob, int depth)
{
	char label[TLIST_LABEL_MAX];
	struct object *cob;
	struct tlist_item *it;

	strlcpy(label, pob->name, sizeof(label));
	if (pob->flags & OBJECT_DATA_RESIDENT) {
		strlcat(label, _(" (resident)"), sizeof(label));
	}
	it = tlist_insert_item(tl, OBJECT_ICON(pob), label, pob);
	it->depth = depth;

	if (!TAILQ_EMPTY(&pob->childs)) {
		it->flags |= TLIST_HAS_CHILDREN;
	}
	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_childs(tl, it)) {
		TAILQ_FOREACH(cob, &pob->childs, cobjs)
			find_objs(tl, cob, depth+1);
	}
	return (it);
}

static void
poll_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *pob = argv[1].p;

	lock_linkage();
	tlist_clear_items(tl);
	find_objs(tl, pob, 0);
	tlist_restore_selections(tl);
	unlock_linkage();
}

static void
poll_input_devs(int argc, union evarg *argv)
{
	extern struct input_devq input_devs;
	extern pthread_mutex_t input_lock;
	struct tlist *tl = argv[0].p;
	struct input *in;

	pthread_mutex_lock(&input_lock);
	tlist_clear_items(tl);
	SLIST_FOREACH(in, &input_devs, inputs) {
		tlist_insert_item(tl, NULL, in->name, in);
	}
	tlist_restore_selections(tl);
	pthread_mutex_unlock(&input_lock);
}

static void
poll_submaps(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct position *po = argv[1].p;
	struct object *ob = po->obj;

	if (ob == NULL)
		return;

	tlist_clear_items(tl);
	tlist_restore_selections(tl);
}

void
position_init(void *p)
{
	struct position *po = p;
	struct window *win;
	struct box *bo;
	struct tlist *tl;

	tool_init(&po->tool, "position", &position_ops, MAPEDIT_TOOL_POSITION);
	po->speed = 1;
	po->center_view = 0;
	po->soft_motion = 1;
	po->pass_through = 0;

	win = TOOL(po)->win = window_new("mapedit-tool-position");
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	window_set_caption(win, _("Position"));
	event_new(win, "window-close", tool_window_close, "%p", po);

	tl = tlist_new(win, TLIST_POLL|TLIST_TREE);
	WIDGET(tl)->flags |= WIDGET_HFILL;
	tlist_prescale(tl, "XXXXXXXXXXXXXXXXX", 5);
	widget_bind(tl, "selected", WIDGET_POINTER, &po->obj);
	event_new(tl, "tlist-poll", poll_objs, "%p", world);

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		struct checkbox *cb;
		struct spinbutton *sb;
		struct combo *com;

		com = combo_new(bo, COMBO_POLL, _("Submap: "));
		event_new(com->list, "tlist-poll", poll_submaps, "%p", po);
		widget_bind(com->list, "selected", WIDGET_POINTER, &po->submap);

		com = combo_new(bo, COMBO_POLL, _("Input device: "));
		event_new(com->list, "tlist-poll", poll_input_devs, NULL);
		widget_bind(com->list, "selected", WIDGET_POINTER,
		    &po->input_dev);
	
		sb = spinbutton_new(bo, _("Speed: "));
		widget_bind(sb, "value", WIDGET_INT, &po->speed);
		spinbutton_set_min(sb, 1);
		spinbutton_set_max(sb, 16);
		spinbutton_set_increment(sb, 1);
		
		cb = checkbox_new(bo, _("Center view around"));
		widget_bind(cb, "state", WIDGET_BOOL, &po->center_view);
		
		cb = checkbox_new(bo, _("Soft motion"));
		widget_bind(cb, "state", WIDGET_BOOL, &po->soft_motion);
		
		cb = checkbox_new(bo, _("Pass through"));
		widget_bind(cb, "state", WIDGET_BOOL, &po->pass_through);
	}
}

static void
position_effect(void *p, struct mapview *mv, struct map *m, struct node *dn)
{
	struct position *po = p;
	struct object *ob = po->obj;
	int dirflags = 0;

	if (ob == NULL)
		return;

	dprintf("position `%s'\n", ob->name);
	object_set_position(ob, mv->map, mv->cx, mv->cy, mv->map->cur_layer);
	object_set_submap(ob, po->submap);
	ob->pos->input = po->input_dev;

	if (po->center_view)
		dirflags |= DIR_CENTER_VIEW;
	if (po->soft_motion)
		dirflags |= DIR_SOFT_MOTION;
	if (po->pass_through)
		dirflags |= DIR_PASS_THROUGH;

	object_set_direction(ob, DIR_S, dirflags, po->speed);
}

static int
position_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	return (-1);
}

