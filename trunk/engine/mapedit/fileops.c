/*	$Csoft: fileops.c,v 1.15 2002/11/22 08:56:52 vedge Exp $	*/

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
#include "command.h"
#include "fileops.h"
#include "mapwin.h"
#include "mapview.h"

void
fileops_new_map(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct textbox *media_tbox = argv[3].p;
	struct textbox *w_tbox = argv[4].p;
	struct textbox *h_tbox = argv[5].p;
	struct window *win;
	struct node *origin;
	struct map *m;
	char *name, *media;
	Uint32 w, h;

	name = textbox_string(name_tbox);
	media = textbox_string(media_tbox);

	if (strcmp(name, "") == 0) {
		text_msg("Error", "No map name given.");
		goto out;
	}
	w = (Uint32)textbox_int(w_tbox);
	h = (Uint32)textbox_int(h_tbox);

	m = emalloc(sizeof(struct map));
	map_init(m, name, strcmp(media, "") == 0 ? NULL : media, MAP_2D);
	map_allocnodes(m, w, h);

	m->defx = w / 2;
	m->defy = h - 2;	/* XXX pref */

	origin = &m->map[m->defy][m->defx];
	origin->flags |= NODE_ORIGIN;

	win = mapwin_new(med, m);
	view_attach(win);
	window_show(win);

	textbox_printf(name_tbox, "");
	textbox_printf(media_tbox, "");

	window_hide(wid->win);
out:
	free(name);
	free(media);
}

void
fileops_save_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	object_save(mv->map);
}

void
fileops_load_map(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct window *win;
	struct map *m;
	char *name;

	name = textbox_string(name_tbox);

	if (strcmp(name, "") == 0) {
		text_msg("Error", "No map name given.");
		goto out;
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

void
fileops_revert_map(int argc, union evarg *argv)
{
	char *path;
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;
	char *s;

	/* Always edit the user copy. */
	s = prop_string(config, "path.user_data_dir");
	asprintf(&path, "%s/maps/%s.m", s, OBJECT(m)->name);
	free(s);

	if (object_load_from(mv->map, path) == 0) {
		mapview_center(mv, m->defx, m->defy);
	}
	free(path);
}

void
fileops_clear_map(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct mapedit *med = mv->med;
	struct map *m = mv->map;
	struct editref *eref;
	
	map_clean(m, med->ref.obj, med->ref.offs,
	    med->node.flags & ~(NODE_ORIGIN|NODE_ANIM), med->ref.flags);
}
