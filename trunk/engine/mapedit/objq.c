/*	$Csoft: objq.c,v 1.27 2002/11/28 07:19:45 vedge Exp $	*/

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

#include <engine/compat/asprintf.h>
#include <engine/engine.h>

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>

#include "mapedit.h"
#include "mapview.h"
#include "objq.h"
#include "fileops.h"

static void	 tilemap_option(int, union evarg *);

static void
tl_objs_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct editobj *eob;

	tlist_clear_items(tl);

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		struct object *ob = eob->pobj;
		struct art *art = ob->art;
		SDL_Surface *icon = NULL;

		if (ob->art != NULL && ob->art->nsprites > 0) {
			icon = ob->art->sprites[0];
		}
		tlist_insert_item(tl, icon, ob->name, ob);
	}

	tlist_restore_selections(tl);
}

static void
tilemap_option(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int opt = argv[2].i;

	switch (opt) {
	case MAPEDIT_TOOL_GRID:
		if (mv->flags & MAPVIEW_GRID) {
			mv->flags &= ~(MAPVIEW_GRID);
		} else {
			mv->flags |= MAPVIEW_GRID;
		}
		break;
	case MAPEDIT_TOOL_PROPS:
		if (mv->flags & MAPVIEW_PROPS) {
			mv->flags &= ~(MAPVIEW_PROPS);
		} else {
			mv->flags |= MAPVIEW_PROPS;
		}
		break;
	case MAPEDIT_TOOL_SHOW_CURSOR:
		if (mv->flags & MAPVIEW_SHOW_CURSOR) {
			mv->flags &= ~(MAPVIEW_SHOW_CURSOR);
		} else {
			mv->flags |= MAPVIEW_SHOW_CURSOR;
		}
		break;
	case MAPEDIT_TOOL_EDIT:
		if (mv->flags & MAPVIEW_EDIT) {
			mv->flags &= ~(MAPVIEW_EDIT);
		} else {
			mv->flags |= MAPVIEW_EDIT;
		}
		break;
	}
}

static void
tl_objs_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct tlist_item *eob_item = argv[2].p;
	struct object *ob = eob_item->p1;
	struct window *win;
	struct region *reg;
	struct mapview *mv;
	struct button *bu;
	static int cury = 85;
	char *wname;

	/* XXX use generic windows for this */
	/* Create a new window for the tile map. */
	win = window_generic_new(88, 264, "mapedit-tmap-%s", ob->name);
	win->rd.x = view->w - 170;
	win->rd.y = cury;
	win->minw = 56;
	win->minh = 117;
	if ((cury += 32) + 264 > view->h) {
		cury = 85;
	}
	window_set_caption(win, "%s", ob->name);

	/* Create the tile map. */
	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, ob->art->map,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_TILEMAP|MAPVIEW_GRID|
	    MAPVIEW_PROPS|MAPVIEW_SHOW_CURSOR,
	    100, 100);
	
	/*
	 * Tools
	 */
	reg = region_new(win, REGION_HALIGN, 0, 0, 93, 10);
	reg->spacing = 1;

	/* Load map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_LOAD_MAP),
	    0,
	    20, 100);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", fileops_revert_map, "%p", mv);

	/* Save map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SAVE_MAP),
	    0,
	    20, 100);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", fileops_save_map, "%p", mv);

	/* Grid */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_GRID),
	    BUTTON_STICKY|BUTTON_PRESSED,
	    20, 100);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed",
	    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_GRID);

	/* Props */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_PROPS),
	    BUTTON_STICKY|BUTTON_PRESSED,
	    20, 100);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed",
	    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_PROPS);
	
	/* Edition */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_EDIT),
	    BUTTON_STICKY,
	    20, 100);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed",
	    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_EDIT);

	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, 10, 100, 90);
	region_attach(reg, mv);

	window_show(win);
}

struct window *
objq_window(struct mapedit *med)
{
	struct window *win;
	struct region *reg;

	win = window_generic_new(215, 140, "mapedit-objq");
	if (win == NULL) {
		return (NULL);		/* Exists */
	}
	window_set_caption(win, "Objects");

	reg = region_new(win, 0, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_objs_poll, "%p", med);
		event_new(tl, "tlist-changed", tl_objs_selected, "%p", med);
	}

	return (win);
}

