/*	$Csoft: position.c,v 1.11 2004/02/20 04:18:11 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapedit.h>

#include <engine/widget/tlist.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/combo.h>
#include <engine/widget/spinbutton.h>

#include <string.h>

static void position_tool_init(void);
static int position_tool_cursor(struct mapview *, SDL_Rect *);
static void position_tool_effect(struct mapview *, struct map *, struct node *);

struct tool position_tool = {
	N_("Position"),
	N_("Assign unique object positions."),
	MAPEDIT_TOOL_POSITION,
	-1,
	position_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	position_tool_effect,
	position_tool_cursor,
	NULL			/* mouse */
};

static int center_view = 0;		/* Center view around? */
static int pass_through = 0;		/* Ignore node movement restrictions */
static void *obj = NULL;		/* Object to position */
static void *projmap = NULL;		/* Projection map to display */
static void *input_dev = NULL;		/* Input device to control object */
static int direction = 0;		/* Angle of velocity vector */
static int velocity = 0;		/* Length of velocity vector */

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

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
	}
	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_children(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
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
poll_projmaps(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = obj, *child;

	if (ob == NULL)
		return;

	tlist_clear_items(tl);
	TAILQ_FOREACH(child, &ob->children, cobjs) {
		if (strcmp(child->type, "map") == 0) {
			tlist_insert_item(tl, OBJECT_ICON(child), child->name,
			    child);
		}
	}
	tlist_restore_selections(tl);
}

static void
position_tool_init(void)
{
	struct window *win;
	struct box *bo;
	struct tlist *tl;

	win = tool_window_new(&position_tool, "mapedit-tool-position");

	tl = tlist_new(win, TLIST_POLL|TLIST_TREE);
	WIDGET(tl)->flags |= WIDGET_HFILL;
	tlist_prescale(tl, "XXXXXXXXXXXXXXXXX", 5);
	widget_bind(tl, "selected", WIDGET_POINTER, &obj);
	event_new(tl, "tlist-poll", poll_objs, "%p", world);

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		struct checkbox *cb;
		struct spinbutton *sb;
		struct combo *com;

		com = combo_new(bo, COMBO_POLL, _(": "));
		event_new(com->list, "tlist-poll", poll_projmaps, NULL);
		widget_bind(com->list, "selected", WIDGET_POINTER, &projmap);

		com = combo_new(bo, COMBO_POLL, _("Input device: "));
		event_new(com->list, "tlist-poll", poll_input_devs, NULL);
		widget_bind(com->list, "selected", WIDGET_POINTER, &input_dev);
	
		sb = spinbutton_new(bo, _("Direction: "));
		widget_bind(sb, "value", WIDGET_INT, &direction);
		spinbutton_set_min(sb, 0);
		spinbutton_set_max(sb, 255);
		spinbutton_set_increment(sb, 16);
		
		sb = spinbutton_new(bo, _("Velocity: "));
		widget_bind(sb, "value", WIDGET_INT, &velocity);
		spinbutton_set_min(sb, 0);
		spinbutton_set_max(sb, 255);
		spinbutton_set_increment(sb, 2);
		
		cb = checkbox_new(bo, _("Center view"));
		widget_bind(cb, "state", WIDGET_BOOL, &center_view);
		
		cb = checkbox_new(bo, _("Pass through"));
		widget_bind(cb, "state", WIDGET_BOOL, &pass_through);
	}
}

static void
position_tool_effect(struct mapview *mv, struct map *m, struct node *dn)
{
	struct object *ob = obj;
	int posflags = 0;

	if (ob == NULL) {
		text_msg(MSG_ERROR, _("No object selected."));
		return;
	}
	if (projmap == NULL) {
		text_msg(MSG_ERROR, _("No projection map was selected."));
		return;
	}

	if (position_set(ob, mv->map, mv->cx, mv->cy, mv->map->cur_layer,
	    projmap) == -1) {
		text_msg(MSG_ERROR, "%s", error_get());
		return;
	}
	ob->pos->input = input_dev;
	ob->pos->flags = 0;

	if (center_view)
		ob->pos->flags |= POSITION_CENTER_VIEW;
	if (pass_through)
		ob->pos->flags |= POSITION_PASS_THROUGH;

	position_set_velvec(ob, direction, velocity);
}

static int
position_tool_cursor(struct mapview *mv, SDL_Rect *rd)
{
	return (-1);
}

