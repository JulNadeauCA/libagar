/*	$Csoft: map_view.c,v 1.1 2002/09/16 16:44:12 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
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

#include <engine/mcconfig.h>

#ifdef DEBUG

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/bitmap.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"
#include "map_view.h"

static const struct monitor_tool_ops map_view_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	map_view_window
};

static void	lookup_object(int, union evarg *);

struct map_view *
map_view_new(struct monitor *mon, int flags)
{
	struct map_view *map_view;

	map_view = emalloc(sizeof(struct map_view));
	map_view_init(map_view, mon, flags);

	return (map_view);
}

void
map_view_init(struct map_view *map_view, struct monitor *mon,
    int flags)
{
	monitor_tool_init(&map_view->tool, "map_view", mon,
	    &map_view_ops);

	map_view->flags = flags;
}

void
map_view_show(int argc, union evarg *argv)
{
	struct monitor *mon = argv[1].p;
	struct map_view *nmv;

	pthread_mutex_lock(&view->lock);
	if (view->rootmap == NULL) {
		text_msg("Error", "No root map.");
		pthread_mutex_unlock(&view->lock);
		return;
	}
	pthread_mutex_unlock(&view->lock);

	nmv = map_view_new(mon, 0);
	map_view_window(nmv);
}

struct window *
map_view_window(void *p)
{
	struct map_view *obr = p;
	struct window *win;
	struct region *reg;
	struct mapview *mv;

	if (view->rootmap == NULL) {
		fatal("no root map");
	}
	
	win = window_new(NULL, "Map view", WINDOW_TITLEBAR,
	    -1, -1, 320, 200, 320, 200);
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	mv = mapview_new(reg, NULL, view->rootmap->map,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_SHOW_CURSOR,
	    100, 100);
	window_show(win);

	return (win);
}

#endif	/* DEBUG */
