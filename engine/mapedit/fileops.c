/*	$Csoft: fileops.c,v 1.1 2002/06/22 20:43:04 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>

#include "mapedit.h"
#include "command.h"
#include "fileops.h"
#include "mapwin.h"

void
fileops_new_map(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	struct textbox *media_tbox = argv[3].p;
	struct textbox *w_tbox = argv[4].p;
	struct textbox *h_tbox = argv[5].p;
	char *name = name_tbox->text;
	char *media = media_tbox->text;
	struct map *m;
	struct window *win;
	Uint32 w, h;
	struct node *origin;

	if (strcmp(name, "") == 0) {
		warning("no map name given\n");
		return;
	}
	w = (Uint32)atoi(w_tbox->text);
	h = (Uint32)atoi(h_tbox->text);

	m = emalloc(sizeof(struct map));
	map_init(m, name, strcmp(media, "") == 0 ? NULL : media, MAP_2D);

	pthread_mutex_lock(&m->lock);
	map_allocnodes(m, w, h);

	m->defx = w / 2;
	m->defy = h - 2;	/* XXX pref */

	origin = &m->map[m->defy][m->defx];
	origin->flags |= NODE_ORIGIN;
	pthread_mutex_unlock(&m->lock);

	win = mapwin_new(med, m);
	view_attach(win);
	window_show(win);

	textbox_printf(name_tbox, "");
	textbox_printf(media_tbox, "");

	window_hide_locked(wid->win);
}

