/*	$Csoft: mapwin.c,v 1.38 2003/01/27 02:19:09 vedge Exp $	*/

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

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>

#include "mapedit.h"
#include "mapview.h"

static void	mapwin_new_view(int, union evarg *);
static void	mapwin_option(int, union evarg *);

static void
mapwin_new_view(int argc, union evarg *argv)
{
	struct mapview *mv, *parent = argv[1].p;
	struct map *m = parent->map;
	struct window *win;
	struct region *reg;

	win = emalloc(sizeof(struct window));
	window_init(win, NULL, WINDOW_SCALE|WINDOW_CENTER, -1, -1,
	    50, 60,
	    50, 60);

	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	mv = mapview_new(reg, m, MAPVIEW_CENTER|MAPVIEW_ZOOM, 100, 100);

	win->focus = WIDGET(mv);

	event_new(win, "window-close", window_generic_detach, "%p", win);

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
	case MAPEDIT_TOOL_NODEEDIT:
		if (mv->node.win->flags & WINDOW_SHOWN) {
			window_hide(mv->node.win);
		} else {
			window_show(mv->node.win);
		}
		break;
	}
}

struct window *
mapwin_new(struct map *m)
{
	struct window *win;
	struct region *reg;
	struct mapview *mv;

	win = emalloc(sizeof(struct window));
	window_init(win, NULL, WINDOW_SCALE|WINDOW_CENTER, -1, -1,
	    60, 70,
	    60, 70);

	/* Map view */
	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, m,
	    MAPVIEW_CENTER|MAPVIEW_EDIT|MAPVIEW_ZOOM|MAPVIEW_PROPS,
	    100, 100);

	/* Tools */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
	region_set_spacing(reg, 1, 1);
	{
		struct button *bu;

		/* Load map */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_LOAD_MAP),
		    0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_revert_map, "%p", mv);

		/* Save map */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_SAVE_MAP),
		    0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_save_map, "%p", mv);

		/* Clear map */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_CLEAR_MAP),
		    0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_clear_map, "%p", mv);

		/* New map view */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_NEW_VIEW),
		    0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", mapwin_new_view, "%p", mv);

		/* Toggle mapview grid */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_GRID),
		    BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_GRID);

		/* Toggle node props */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_PROPS),
		    BUTTON_STICKY, -1, -1);
		widget_set_bool(bu, "state", 1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_PROPS);
		
		/* Toggle node edition */
		bu = button_new(reg, NULL,
		    SPRITE(&mapedit, MAPEDIT_TOOL_NODEEDIT),
		    BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    mapwin_option, "%p, %i", mv, MAPEDIT_TOOL_NODEEDIT);
		mv->node.button = bu;
	}

	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	region_set_spacing(reg, 0, 0);
	{
		region_attach(reg, mv);
		win->focus = WIDGET(mv);
	}
	return (win);
}

