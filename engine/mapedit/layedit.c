/*	$Csoft: layedit.c,v 1.25 2005/01/05 04:44:04 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include "layedit.h"

static void layedit_init(struct tool *);

const struct tool layedit_tool = {
	N_("Layer editor"),
	N_("Edit the layer information."),
	LAYER_EDITOR_ICON,
	-1,
	layedit_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

/* Update the list of layers. */
void
layedit_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct map *m = argv[1].p;
	int i;
	
	tlist_clear_items(tl);
	for (i = 0; i < m->nlayers; i++) {
		char label[TLIST_LABEL_MAX];
		struct map_layer *layer = &m->layers[i];

		if (layer->visible) {
			snprintf(label, sizeof(label), _("%s (visible%s)\n"),
			    layer->name,
			    (i == m->cur_layer) ? _(", editing") : "");
		} else {
			snprintf(label, sizeof(label), _("%s (invisible%s)\n"),
			    layer->name,
			    (i == m->cur_layer) ? _(", editing") : "");
		}

		tlist_insert_item(tl, NULL, label, layer);
	}
	tlist_restore_selections(tl);

	/* XXX load/save hack */
	if (m->cur_layer >= m->nlayers)
		m->cur_layer--;
}

static void
push_layer(int argc, union evarg *argv)
{
	char name[MAP_LAYER_NAME_MAX];
	struct map *m = argv[1].p;
	struct textbox *tb = argv[2].p;
	
	textbox_copy_string(tb, name, sizeof(name));

	if (map_push_layer(m, name) != 0) {
		text_msg(MSG_ERROR, "%s", error_get());
	} else {
		textbox_printf(tb, NULL);
	}
}

static void
selected_layer(int argc, union evarg *argv)
{
	struct textbox *rename_tb = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct map_layer *layer = it->p1;

	textbox_printf(rename_tb, "%s", layer->name);
}

static void
rename_layer(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;
	struct textbox *tb = argv[2].p;
	struct tlist *tl = argv[3].p;
	struct tlist_item *it;
	struct map_layer *layer;

	if ((it = tlist_item_selected(tl)) == NULL) {
		return;
	}
	layer = it->p1;

	textbox_copy_string(tb, layer->name, sizeof(layer->name));
	textbox_printf(tb, NULL);
}

static void
pop_layer(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;
	int destructive = argv[2].i;
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
					Free(r, M_MAP_NODEREF);
				}
			}
		}
	}
}

/* Toggle edition of a layer. */
static void
edit_layer(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct tlist_item *it;
	int i = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			m->cur_layer = i;
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
	struct map *m = argv[1].p;
	struct tlist *tl = argv[2].p;
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
layedit_init(struct tool *t)
{
	struct map *m = t->p;
	struct window *win;
	struct hbox *hb;
	struct textbox *rename_tb;
	struct tlist *tl;
#ifdef DEBUG
	if (!OBJECT_TYPE(m, "map"))
		fatal("type");
#endif

	win = tool_window(t, "mapedit-layedit");
	window_set_caption(win, _("%s layers"), OBJECT(m)->name);
	window_set_spacing(win, 2);
	
	tl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl, TLIST_POLL);
	tlist_prescale(tl, _("Layer NN (visible, editing)    "), 5);
	event_new(tl, "tlist-poll", layedit_poll, "%p", m);
	event_new(tl, "tlist-dblclick", edit_layer, "%p, %p", m, tl);
	
	hb = hbox_new(win, HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct textbox *tb;
		struct button *bu;

		tb = textbox_new(hb, _("New layer: "));
		event_new(tb, "textbox-return", push_layer, "%p, %p", m, tb);

		bu = button_new(hb, _("Push"));
		button_set_padding(bu, 6);
		event_new(bu, "button-pushed", push_layer, "%p, %p", m, tb);
	}

	hb = hbox_new(win, HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct button *bu;

		rename_tb = textbox_new(hb, "-> ");
		event_new(rename_tb, "textbox-return", rename_layer,
		    "%p, %p, %p", m, rename_tb, tl);

		bu = button_new(hb, _("Rename"));
		button_set_padding(bu, 6);
		event_new(bu, "button-pushed", rename_layer,
		    "%p, %p, %p", m, rename_tb, tl);
		event_new(tl, "tlist-selected", selected_layer, "%p",
		    rename_tb);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	hbox_set_padding(hb, 0);
	{
		struct button *bu;

		bu = button_new(hb, _("Pop"));
		event_new(bu, "button-pushed", pop_layer, "%p, %i", m, 0);
		bu = button_new(hb, _("Destructive Pop"));
		event_new(bu, "button-pushed", pop_layer, "%p, %i", m, 1);
		bu = button_new(hb, _("Edit"));
		event_new(bu, "button-pushed", edit_layer, "%p, %p", m, tl);
		bu = button_new(hb, _("Show/hide"));
		event_new(bu, "button-pushed", mask_layer, "%p, %p", m, tl);
	}

	object_attach(win, tl);
}

#endif	/* EDITION */
