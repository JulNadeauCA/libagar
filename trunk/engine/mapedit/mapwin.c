/*	$Csoft: mapwin.c,v 1.16 2002/09/12 09:35:23 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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

#include "mapedit.h"
#include "command.h"
#include "mapwin.h"
#include "mapview.h"
#include "fileops.h"
#include "tilestack.h"

static void	mapwin_new_view(int, union evarg *);
static void	mapwin_option(int, union evarg *);

static void
mapwin_new_view(int argc, union evarg *argv)
{
	char caption[4096];
	struct mapview *mv, *parent = argv[1].p;
	struct mapedit *med = parent->med;
	struct map *m = parent->map;
	struct window *win;
	struct region *reg;

	sprintf(caption, "%s view (%dx%d)",
	    OBJECT(m)->name, m->mapw, m->maph);
	win = emalloc(sizeof(struct window));
	window_init(win, NULL, caption, WINDOW_SOLID,
	    96, 96, 375, 293, 100, 64);

	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	mv = mapview_new(reg, med, m,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_SHOW_CURSOR, 100, 100);

	win->focus = WIDGET(mv);

	view_attach(win);
	window_show(win);
}

static void
mapwin_option(int argc, union evarg *argv)
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
	}
}

struct window *
mapwin_new(struct mapedit *med, struct map *m)
{
	char caption[4096];
	struct window *win;
	struct region *reg;
	struct mapview *mv;
	struct button *bu;
	struct tilestack *ts;

	sprintf(caption, "%s edition (%dx%d)",
	    OBJECT(m)->name, m->mapw, m->maph);

	win = emalloc(sizeof(struct window));
	window_init(win, NULL, caption, WINDOW_SOLID|WINDOW_CENTER,
	    0, 0, 420, 300, 375, 293);

	/* Map view */
	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, m,
	    MAPVIEW_CENTER|MAPVIEW_EDIT|MAPVIEW_ZOOM|MAPVIEW_SHOW_CURSOR|
	    MAPVIEW_PROPS,
	    100, 100);

	/*
	 * Tools
	 */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -25);
	reg->spacing = 1;

	/* Load map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_LOAD_MAP),
	    0, -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0, fileops_revert_map, "%p", mv);

	/* Save map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SAVE_MAP),
	    0, -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0, fileops_save_map, "%p", mv);

	/* Clear map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_CLEAR_MAP), 0,
	    -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0, fileops_clear_map, "%p", mv);

	/* New map view */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_NEW_VIEW), 0,
	    -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0, mapwin_new_view, "%p", mv);

	/* Grid */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_GRID),
	    BUTTON_STICKY, -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0,
	    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_GRID);

	/* Props */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_PROPS),
	    BUTTON_STICKY|BUTTON_PRESSED, -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0,
	    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_PROPS);

	/* Show/hide cursor */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SHOW_CURSOR),
	    BUTTON_STICKY|BUTTON_PRESSED, -1, -1);
	WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
	event_new(bu, "button-pushed", 0,
	    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_SHOW_CURSOR);


	/* Tile stack */
	reg = region_new(win, REGION_VALIGN, 0, 10, -TILEW, 90);
	reg->spacing = 1;
	ts = tilestack_new(reg, TILESTACK_VERT, 100, 100, mv);

	/*
	 * Map view
	 */
	reg = region_new(win, REGION_HALIGN, 10, 10, 90, 90);
	region_attach(reg, mv);

	win->focus = WIDGET(mv);

	return (win);
}

