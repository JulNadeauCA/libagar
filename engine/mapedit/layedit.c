/*	$Csoft: layedit.c,v 1.6 2003/05/07 00:47:54 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/snprintf.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>

#include "mapedit.h"
#include "mapview.h"

static void
layedit_close_win(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->layed.trigger, "state", 0);
}

static void
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

		snprintf(label, sizeof(label), "%s (%svisible%s)\n",
		    layer->name, !layer->visible ? "in" : "",
		    (i == m->cur_layer) ? ", editing" : "");
		tlist_insert_item(tl, NULL, label, layer);
	}
	tlist_restore_selections(tl);

	/* XXX load/save hack */
	if (m->cur_layer >= m->nlayers) {
		m->cur_layer--;
	}
}

static void
layedit_push(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	char *name;
	
	name = textbox_string(name_tbox);
	if (strcmp("", name) == 0) {
		free(name);
		name = NULL;					/* Default */
	}
	if (map_push_layer(mv->map, name) != 0) {
		text_msg("Error", "%s", error_get());
	} else {
		textbox_printf(name_tbox, "");			/* Clear */
	}
	Free(name);
}

static void
layedit_update_name(int argc, union evarg *argv)
{
	struct textbox *tb = argv[2].p;
	struct tlist_item *it = argv[3].p;
	int state = argv[4].i;
	struct map_layer *lay;

	if (!state) {
		return;
	}
	lay = it->p1;
	textbox_printf(tb, "%s", lay->name);
}

static void
layedit_rename(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct tlist_item *it;
	struct map_layer *lay;

	if ((it = tlist_item_selected(mv->layed.layers_tl)) == NULL) {
		return;
	}
	lay = it->p1;

	free(lay->name);
	lay->name = textbox_string(name_tbox);
	
	textbox_printf(name_tbox, "");
}

static void
layedit_pop(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (mv->map->cur_layer == mv->map->nlayers-1 &&
	    mv->map->nlayers > 1) {
		mv->map->cur_layer--;
	}
	map_pop_layer(mv->map);
}

static void
layedit_edit(int argc, union evarg *argv)
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
	text_msg("Error", "No layer is selected");
}

static void
layedit_visible(int argc, union evarg *argv)
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
	text_msg("Error", "No layer is selected");
}

void
layedit_init(struct mapview *mv)
{
	struct map *m = mv->map;
	struct window *win;
	struct region *reg;
	struct textbox *rename_tb;

	win = window_generic_new(268, 346, "mapedit-lay-%s-%s",
	    OBJECT(mv)->name, OBJECT(m)->name);
	if (win == NULL) {
		return;						/* Exists */
	}
	window_set_caption(win, "%s layers", OBJECT(m)->name);
	window_set_min_geo(win, 175, 160);
	event_new(win, "window-close", layedit_close_win, "%p", mv);
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct textbox *tb;
		struct button *bu;

		tb = textbox_new(reg, "New layer: ", 0, 75, -1);
		event_new(tb, "textbox-return", layedit_push, "%p, %p", mv, tb);

		bu = button_new(reg, "Push", NULL, 0, 25, -1);
		event_new(bu, "button-pushed", layedit_push, "%p, %p", mv, tb);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;

		rename_tb = textbox_new(reg, "-> ", 0, 75, -1);
		event_new(rename_tb, "textbox-return",
		    layedit_rename, "%p, %p", mv, rename_tb);

		bu = button_new(reg, "Rename", NULL, 0, 25, -1);
		event_new(bu, "button-pushed",
		    layedit_rename, "%p, %p", mv, rename_tb);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;

		bu = button_new(reg, "Pop", NULL, 0, 30, -1);
		event_new(bu, "button-pushed", layedit_pop, "%p", mv);
		
		bu = button_new(reg, "Edit", NULL, 0, 30, -1);
		event_new(bu, "button-pushed", layedit_edit, "%p", mv);
		
		bu = button_new(reg, "Show/hide", NULL, 0, 40, -1);
		event_new(bu, "button-pushed", layedit_visible, "%p", mv);
	}

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		mv->layed.layers_tl = tlist_new(reg, 100, 100, 
		    TLIST_POLL|TLIST_DBLCLICK);
		event_new(mv->layed.layers_tl, "tlist-poll",
		    layedit_poll, "%p", mv);
		event_new(mv->layed.layers_tl, "tlist-changed",
		    layedit_update_name, "%p, %p", mv, rename_tb);
		event_new(mv->layed.layers_tl, "tlist-dblclick",
		    layedit_edit, "%p", mv);
	}

	mv->layed.win = win;
}
