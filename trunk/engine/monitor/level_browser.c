/*	$Csoft: level_browser.c,v 1.6 2003/01/27 08:05:17 vedge Exp $	*/

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

#ifdef DEBUG

#include <engine/world.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static void
tl_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob;

	tlist_clear_items(tl);
	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		if (strcmp(ob->type, "map") != 0)
			continue;
		tlist_insert_item(tl, NULL, ob->name, ob);
	}
	pthread_mutex_unlock(&world->lock);
	tlist_restore_selections(tl);
}

struct window *
level_browser_window(void)
{
	struct window *win;
	struct region *reg;

	win = window_generic_new(320, 200, "monitor-level-browser");
	if (win == NULL)
		return (NULL);		/* Exists */
	window_set_caption(win, "Levels");
	
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	{
		struct tlist *tl;
	
		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_poll, NULL);
	}

	return (win);
}

#endif	/* DEBUG */
