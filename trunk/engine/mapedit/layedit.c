/*	$Csoft: layedit.c,v 1.19 2004/03/10 03:12:37 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

#include <config/edition.h>
#ifdef EDITION

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/textbox.h>

#include "mapedit.h"
#include "mapview.h"

static void
close_window(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->layed.trigger, "state", 0);
	window_hide(win);
}

/* Update the list of layers. */
void
layedit_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;
	int i;
	
	tlist_clear_items(tl);
	for (i = 0; i < m->nlayers; i++) {
		struct map_layer *layer = &m->layers[i];
		char label[TLIST_LABEL_MAX];

		if (layer->visible) {
			snprintf(label, sizeof(label), _("%s (visible %s)\n"),
			    layer->name,
			    (i == m->cur_layer) ? _(", editing") : "");
		} else {
			snprintf(label, sizeof(label), _("%s (invisible %s)\n"),
			    layer->name,
			    (i == m->cur_layer) ? _(", editing") : "");
		}

		tlist_insert_item(tl, NULL, label, layer);
	}
	tlist_restore_selections(tl);

	/* XXX load/save hack */
	if (m->cur_layer >= m->nlayers) {
		m->cur_layer--;
	}
}

static void
push_layer(int argc, union evarg *argv)
{
	char name[MAP_LAYER_NAME_MAX];
	struct mapview *mv = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	
	textbox_copy_string(name_tbox, name, sizeof(name));

	if (map_push_layer(mv->map, name) != 0) {
		text_msg(MSG_ERROR, "%s", error_get());
	} else {
		textbox_printf(name_tbox, " ");
	}
}

static void
select_layer(int argc, union evarg *argv)
{
	struct textbox *rename_tb = argv[2].p;
	struct tlist_item *it = argv[3].p;
	int selected = argv[4].i;
	struct map_layer *lay;

	if (!selected) {
		return;
	}
	lay = it->p1;
	textbox_printf(rename_tb, "%s", lay->name);
}

static void
rename_layer(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct tlist_item *it;
	struct map_layer *lay;

	if ((it = tlist_item_selected(mv->layed.layers_tl)) == NULL) {
		return;
	}
	lay = it->p1;

	textbox_copy_string(name_tbox, lay->name, sizeof(lay->name));
	textbox_printf(name_tbox, " ");
}

static void
pop_layer(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int destructive = argv[2].i;
	struct map *m = mv->map;
	int x, y;

	if (m->cur_layer == m->nlayers-1 &&
	    m->nlayers > 1) {
		m->cur_layer--;
	}
	map_pop_layer(m);

	if (destructive) {
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				struct node *node = &m->map[y][x];
				struct noderef *r, *nr;

				for (r = TAILQ_FIRST(&node->nrefs);
				     r != TAILQ_END(&node->nrefs);
				     r = nr) {
					nr = TAILQ_NEXT(r, nrefs);
					if (r->layer != m->cur_layer-1)
						continue;

					TAILQ_REMOVE(&node->nrefs, r, nrefs);
					noderef_destroy(m, r);
				}
			}
		}
	}
}

/* Toggle edition of a layer. */
static void
edit_layer(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct tlist *tl = mv->layed.layers_tl;
	struct tlist_item *it;
	int i = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			mv->map->cur_layer = i;
			return;
		}
		i++;
	}
	text_msg(MSG_ERROR, _("No layer is selected."));
}

/* Toggle visibility of a layer. */
static void
mask_layer(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct tlist *tl = mv->layed.layers_tl;
	struct tlist_item *it;
	int i = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct map_layer *lay = it->p1;

		if (it->selected) {
			lay->visible = !lay->visible;
			return;
		}
		i++;
	}
	text_msg(MSG_ERROR, _("No layer is selected."));
}

void
layedit_init(struct mapview *mv)
{
	struct map *m = mv->map;
	struct window *win;
	struct hbox *hb;
	struct textbox *rename_tb;
	struct tlist *tl;

	win = window_new(NULL);
	window_set_caption(win, _("%s layers"), OBJECT(m)->name);
	window_set_spacing(win, 2);
	event_new(win, "window-close", close_window, "%p", mv);
	
	hb = hbox_new(win, HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct textbox *tb;
		struct button *bu;

		tb = textbox_new(hb, _("New layer: "));
		event_new(tb, "textbox-return", push_layer, "%p, %p", mv, tb);

		bu = button_new(hb, _("Push"));
		button_set_padding(bu, 6);
		event_new(bu, "button-pushed", push_layer, "%p, %p", mv, tb);
	}

	hb = hbox_new(win, HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct button *bu;

		rename_tb = textbox_new(hb, "-> ");
		event_new(rename_tb, "textbox-return", rename_layer, "%p, %p",
		    mv, rename_tb);

		bu = button_new(hb, _("Rename"));
		button_set_padding(bu, 6);
		event_new(bu, "button-pushed", rename_layer, "%p, %p", mv,
		    rename_tb);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct button *bu;

		bu = button_new(hb, _("Pop"));
		event_new(bu, "button-pushed", pop_layer, "%p, %i", mv, 0);
		bu = button_new(hb, _("Destructive Pop"));
		event_new(bu, "button-pushed", pop_layer, "%p, %i", mv, 1);
		bu = button_new(hb, _("Edit"));
		event_new(bu, "button-pushed", edit_layer, "%p", mv);
		bu = button_new(hb, _("Show/hide"));
		event_new(bu, "button-pushed", mask_layer, "%p", mv);
	}

	tl = tlist_new(win, TLIST_POLL|TLIST_DBLCLICK);
	tlist_prescale(tl, _("Layer NN (visible, editing)    "), 5);
	event_new(tl, "tlist-poll", layedit_poll, "%p", mv);
	event_new(tl, "tlist-changed", select_layer, "%p, %p", mv, rename_tb);
	event_new(tl, "tlist-dblclick", edit_layer, "%p", mv);

	mv->layed.layers_tl = tl;
	mv->layed.win = win;
}

void
layedit_destroy(struct mapview *mv)
{
	view_detach(mv->layed.win);
}

#endif	/* EDITION */
