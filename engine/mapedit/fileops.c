/*	$Csoft: fileops.c,v 1.28 2003/01/27 08:00:00 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc
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
#include "mapview.h"

struct window *
fileops_new_map_window(void)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox, *w_tbox, *h_tbox;
	struct button *button;

	win = window_new("mapedit-new-map-dialog", WINDOW_CENTER,
	     0, 0, 380, 162, 187, 162);
	window_set_caption(win, "Create a map");
	window_set_spacing(win, 0, 10);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		name_tbox = textbox_new(reg, "Name: ", 0, 100, -1);
		win->focus = WIDGET(name_tbox);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		w_tbox = textbox_new(reg, "W: ", 0, 50, -1);
		textbox_printf(w_tbox, "64");
		h_tbox = textbox_new(reg, "H: ", 0, 50, -1);
		textbox_printf(h_tbox, "64");
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		button = button_new(reg, "Ok", NULL, BUTTON_NOFOCUS,
		    50, 100);
		event_new(button, "button-pushed", fileops_new_map,
		    "%p, %p, %p", name_tbox, w_tbox, h_tbox);
		event_new(name_tbox, "textbox-return", fileops_new_map,
		    "%p, %p, %p", name_tbox, w_tbox, h_tbox);
		
		button = button_new(reg, "Cancel", NULL, BUTTON_NOFOCUS,
		    50, 100);
		event_new(button, "button-pushed", window_generic_hide,
		    "%p", win);
	}

	return (win);
}

struct window *
fileops_load_map_window(void)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox;
	struct button *button;

	win = window_new("mapedit-load-map-dialog", WINDOW_CENTER, -1, -1,
	    356, 105, 100, 105);
	window_set_caption(win, "Load a map");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		name_tbox = textbox_new(reg, "Name: ", 0, 100, -1);
		win->focus = WIDGET(name_tbox);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		button = button_new(reg, "OK", NULL, BUTTON_NOFOCUS, 50, 100);
		event_new(button, "button-pushed", fileops_load_map,
		    "%p", name_tbox);
		event_new(name_tbox, "textbox-return", fileops_load_map,
		    "%p", name_tbox);
		
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
	struct textbox *name_tbox = argv[1].p;
	struct textbox *w_tbox = argv[2].p;
	struct textbox *h_tbox = argv[3].p;
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
	map_init(m, MAP_2D, name, NULL);
	map_alloc_nodes(m, w, h);

	m->defx = w / 2;
	m->defy = h - 2;	/* XXX pref */

	origin = &m->map[m->defy][m->defx];
	origin->flags |= NODE_ORIGIN;

	win = mapwin_new(m);
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
	struct textbox *name_tbox = argv[1].p;
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
	map_init(m, MAP_2D, name, NULL);

	object_load(m);

	win = mapwin_new(m);
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
	Asprintf(&path, "%s/map/%s.map", udatadir, OBJECT(m)->name);

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
	struct map *m = mv->map;
	struct editref *eref;
	Uint32 x, y, orx = 0, ory = 0;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];

			if (node->flags & NODE_ORIGIN) {
				orx = x;
				ory = y;
			}
			node_destroy(node, x, y);
			node_init(node, x, y);
		}
	}

	/* Reset the origin. */
	if (orx > 0 && ory > 0) {
		m->map[orx][ory].flags |= NODE_ORIGIN;
	}
}

