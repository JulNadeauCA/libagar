/*	$Csoft: fileops.c,v 1.20 2002/12/14 11:28:56 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
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

#include <engine/compat/asprintf.h>
#include <engine/engine.h>

#include <engine/map.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/text.h>

#include "mapedit.h"
#include "fileops.h"
#include "mapwin.h"
#include "mapview.h"

/* Create the `new map' dialog. */
struct window *
fileops_new_map_window(struct mapedit *med)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox, *w_tbox, *h_tbox;
	struct button *button;

	win = window_new("mapedit-new-map-dialog", WINDOW_CENTER,
	     0, 0, 380, 162, 187, 162);
	window_set_caption(win, "Create a map");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 30);
	{
		name_tbox = textbox_new(reg, "Name: ", 0, 100, 100);
		win->focus = WIDGET(name_tbox);
	}

	reg = region_new(win, REGION_HALIGN, 0, 30, 100, 40);
	{
		w_tbox = textbox_new(reg, "W: ", 0, 50, 100);
		textbox_printf(w_tbox, "64");
		h_tbox = textbox_new(reg, "H: ", 0, 50, 100);
		textbox_printf(h_tbox, "64");
	}

	reg = region_new(win, REGION_HALIGN, 0, 70, 100, 30);
	{
		button = button_new(reg, "OK", NULL, 0, 50, 100);
		event_new(button, "button-pushed", fileops_new_map,
		    "%p, %p, %p, %p",
		    med, name_tbox, w_tbox, h_tbox);
		event_new(name_tbox, "textbox-return", fileops_new_map,
		    "%p, %p, %p, %p",
		    med, name_tbox, w_tbox, h_tbox);
		
		button = button_new(reg, "Cancel", NULL, 0, 50, 100);
		event_new(button, "button-pushed", window_generic_hide,
		    "%p",  win);
	}

	return (win);
}

/* Create the `load map' dialog. */
struct window *
fileops_load_map_window(struct mapedit *med)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox;
	struct button *button;

	win = window_new("mapedit-load-map-dialog", WINDOW_CENTER, -1, -1,
	    356, 105, 100, 105);
	window_set_caption(win, "Load a map");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 50);
	{
		name_tbox = textbox_new(reg, "Name: ", 0, 100, 100);
		win->focus = WIDGET(name_tbox);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, 50, 100, 50);
	{
		button = button_new(reg, "OK", NULL, 0, 50, 100);
		WIDGET(button)->flags |=
		    WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(button, "button-pushed", fileops_load_map,
		    "%p, %p", med, name_tbox);
		event_new(name_tbox, "textbox-return", fileops_load_map,
		    "%p, %p", med, name_tbox);
		
		button = button_new(reg, "Cancel", NULL, 0, 50, 100);
		event_new(button, "button-pushed", window_generic_hide,
		    "%p",  win);
	}

	return (win);
}

/* Create a new map. */
void
fileops_new_map(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct textbox *w_tbox = argv[3].p;
	struct textbox *h_tbox = argv[4].p;
	struct window *win;
	struct node *origin;
	struct map *m;
	char *name, *path;
	Uint32 w, h;

	name = textbox_string(name_tbox);

	if (strcmp(name, "") == 0) {				/* Not empty? */
		text_msg("Error", "No map name given.");
		goto out;
	}
	path = object_path(name, "map");			/* Existing? */
	if (path != NULL) {
		free(path);
		text_msg("Error", "Existing map (%s).", path);
		goto out;
	}
	w = (Uint32)textbox_int(w_tbox);
	h = (Uint32)textbox_int(h_tbox);

	m = emalloc(sizeof(struct map));
	map_init(m, name, NULL, MAP_2D);
	map_alloc_nodes(m, w, h);

	m->defx = w / 2;
	m->defy = h - 2;	/* XXX pref */

	origin = &m->map[m->defy][m->defx];
	origin->flags |= NODE_ORIGIN;

	win = mapwin_new(med, m);
	view_attach(win);
	window_show(win);

	textbox_printf(name_tbox, "");
	window_hide(wid->win);
out:
	free(name);
}

/* Save the map to the default location. */
void
fileops_save_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	object_save(mv->map);
}

/* Load the map from disk. */
void
fileops_load_map(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct window *win;
	struct map *m;
	char *name;
	char *path;

	name = textbox_string(name_tbox);

	if (strcmp(name, "") == 0) {				/* Not empty? */
		text_msg("Error", "No map name given.");
		goto out;
	}
	path = object_path(name, "map");			/* Existing? */
	if (path == NULL) {
		text_msg("Error", "%s.", error_get());
		goto out;
	} else {
		free(path);
	}
	
	m = emalloc(sizeof(struct map));
	map_init(m, name, NULL, MAP_2D);

	object_load(m);

	win = mapwin_new(med, m);
	view_attach(win);
	window_show(win);

	textbox_printf(name_tbox, "");

	window_hide(wid->win);
out:
	free(name);
}

/* Revert to the map on disk. */
void
fileops_revert_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;
	char *path, *udatadir;

	udatadir = prop_get_string(config, "path.user_data_dir");

	/* Always edit the user copy. */
	asprintf(&path, "%s/maps/%s.map", udatadir, OBJECT(m)->name);

	if (object_load_from(mv->map, path) == 0) {
		mapview_center(mv, m->defx, m->defy);
	} else {
		text_msg("Error", "Could not revert: %s", error_get());
	}
	free(path);
}

/* Clear all nodes on a map. */
void
fileops_clear_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct mapedit *med = mv->med;
	struct map *m = mv->map;
	struct editref *eref;
	Uint32 x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];

			node_destroy(node, x, y);
			node_init(node, x, y);
			node->flags = med->node.flags & ~(NODE_ORIGIN);

			switch (med->ref.type) {
			case NODEREF_SPRITE:
				node_add_sprite(node, med->ref.obj,
				    med->ref.offs);
				break;
			case NODEREF_ANIM:
				node_add_anim(node, med->ref.obj,
				    med->ref.offs, NODEREF_ANIM_AUTO);
				break;
			default:
				fatal("bad reference type\n");
				break;
			}
		}
	}
	
	/* Reset the origin. */
	m->map[m->defy][m->defx].flags |= NODE_ORIGIN;
}

