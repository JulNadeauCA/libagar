/*	$Csoft: mapedit.c,v 1.160 2003/03/26 10:04:15 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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
#include <engine/version.h>
#include <engine/map.h>
#include <engine/world.h>
#include <engine/view.h>
#include <engine/prop.h>

#include <stdlib.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/text.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "mapview.h"

#include "tool/tool.h"
#include "tool/stamp.h"
#include "tool/eraser.h"
#include "tool/magnifier.h"
#include "tool/resize.h"
#include "tool/propedit.h"
#include "tool/select.h"
#include "tool/shift.h"
#include "tool/merge.h"
#include "tool/fill.h"
#include "tool/flip.h"

static const struct object_ops mapedit_ops = {
	mapedit_destroy,
	mapedit_load,
	mapedit_save
};

struct mapedit	mapedit;
int		mapedition = 0;

static const struct tools_ent {
	struct tool	**p;
	size_t		  size;
	void		(*init)(void *);
} tools[] = {
	{ &mapedit.tools[MAPEDIT_STAMP], sizeof(struct stamp),
	    stamp_init },
	{ &mapedit.tools[MAPEDIT_ERASER], sizeof(struct eraser),
	    eraser_init },
	{ &mapedit.tools[MAPEDIT_MAGNIFIER], sizeof(struct magnifier),
	    magnifier_init },
	{ &mapedit.tools[MAPEDIT_RESIZE], sizeof(struct resize),
	    resize_init },
	{ &mapedit.tools[MAPEDIT_PROPEDIT], sizeof(struct propedit),
	    propedit_init },
	{ &mapedit.tools[MAPEDIT_SELECT], sizeof(struct select),
	    select_init },
	{ &mapedit.tools[MAPEDIT_SHIFT], sizeof(struct shift),
	    shift_init },
	{ &mapedit.tools[MAPEDIT_MERGE], sizeof(struct merge),
	    merge_init },
	{ &mapedit.tools[MAPEDIT_FILL], sizeof(struct fill),
	    fill_init },
	{ &mapedit.tools[MAPEDIT_FLIP], sizeof(struct flip),
	    flip_init }
};
static const int ntools = sizeof(tools) / sizeof(tools[0]);

static void
mapedit_select_tool(int argc, union evarg *argv)
{
	struct tool *newtool = argv[1].p;

	if (mapedit.curtool == newtool) {
		if (newtool->win != NULL) {
			window_hide(newtool->win);
		}
		mapedit.curtool = NULL;
		return;
	}

	if (mapedit.curtool != NULL) {
		widget_set_bool(mapedit.curtool->button, "state", 0);
		if (mapedit.curtool->win != NULL &&
		   (mapedit.curtool->win->flags & WINDOW_SHOWN)) {
			window_hide(mapedit.curtool->win);
		}
	}

	mapedit.curtool = newtool;

	if (newtool->win != NULL) {
		window_show(newtool->win);
	}
}

void
mapedit_init(void)
{
	const int xdiv = 100, ydiv = 14;
	struct window *win;
	struct region *reg;
	struct button *button;
	struct mapedit *med = &mapedit;
	int i;

	object_init(&med->obj, "map-editor", "map-editor", "mapedit",
	    OBJECT_ART|OBJECT_CANNOT_MAP|OBJECT_STATIC, &mapedit_ops);
	med->curtool = NULL;
	med->src_node = NULL;
	map_init(&med->copybuf, "mapedit-copybuf", NULL);

	prop_set_uint32(med, "default-map-width", 64);
	prop_set_uint32(med, "default-map-height", 32);
	prop_set_uint32(med, "default-brush-width", 5);
	prop_set_uint32(med, "default-brush-height", 5);

	mapedition = 1;

	/* Initialize the map edition tools. */
	for (i = 0; i < ntools; i++) {
		const struct tools_ent *toolent = &tools[i];

		*toolent->p = Malloc(toolent->size);
		toolent->init(*toolent->p);
		world_attach(*toolent->p);
	}
	
	/* Load the settings. */
	object_load(&mapedit, NULL);

	/* Create the dialogs. */
	med->win.objlist = objq_window();
	med->win.new_map = fileops_new_map_window();
	med->win.load_map = fileops_load_map_window();
	
	/* Create the toolbar. */
	win = med->win.toolbar = window_new("mapedit-toolbar", 0,
	    0, 0,
	    94, 153,
	    63, 126);
	window_set_caption(win, "Tools");
	window_set_spacing(win, 0, 0);

	reg = region_new(win, REGION_VALIGN, 0, 0, 50, 100);
	region_set_spacing(reg, 0, 0);
	{
		button = button_new(reg, NULL,			/* New map */
		    SPRITE(med, MAPEDIT_TOOL_NEW_MAP), 0, xdiv, ydiv);
		event_new(button, "button-pushed", window_generic_show,
		    "%p", med->win.new_map);
		win->focus = WIDGET(button);
		
		button = button_new(reg, NULL,			/* Obj list */
		    SPRITE(med, MAPEDIT_TOOL_OBJLIST), 0, xdiv, ydiv);
		event_new(button, "button-pushed", window_generic_show,
		    "%p", med->win.objlist);
	
		button = med->tools[MAPEDIT_STAMP]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_STAMP),
		    BUTTON_NOFOCUS|BUTTON_STICKY, xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_STAMP]);

		button = med->tools[MAPEDIT_MAGNIFIER]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_MAGNIFIER),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_MAGNIFIER]);
		
		button = med->tools[MAPEDIT_SELECT]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SELECT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_SELECT]);
		
		button = med->tools[MAPEDIT_MERGE]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_MERGE),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_MERGE]);
	}

	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	region_set_spacing(reg, 0, 0);
	{
		button = button_new(reg, NULL,			/* Load map */
		    SPRITE(med, MAPEDIT_TOOL_LOAD_MAP), 0, xdiv, ydiv);
		win->focus = WIDGET(button);
		event_new(button, "button-pushed", window_generic_show,
		    "%p", med->win.load_map);

		button = med->tools[MAPEDIT_ERASER]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_ERASER),
		    BUTTON_STICKY|BUTTON_NOFOCUS,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_ERASER]);

		button = med->tools[MAPEDIT_RESIZE]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_RESIZE),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_RESIZE]);

		button = med->tools[MAPEDIT_PROPEDIT]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_PROPEDIT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_PROPEDIT]);

		button = med->tools[MAPEDIT_SHIFT]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SHIFT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_SHIFT]);
		
		button = med->tools[MAPEDIT_FILL]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_FILL),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_FILL]);
		
		button = med->tools[MAPEDIT_FLIP]->button =
		    button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_FLIP),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools[MAPEDIT_FLIP]);
	}
	
	window_show(med->win.toolbar);
	window_show(med->win.objlist);
	
	world_attach(med);
}

void
mapedit_destroy(void *p)
{
	struct mapedit *med = p;
	int i;

	map_destroy(&med->copybuf);

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		object_destroy(med->tools[i]);
		free(med->tools[i]);
	}
}

int
mapedit_load(void *p, struct netbuf *buf)
{
	int i;

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		struct tool *tool = mapedit.tools[i];

		if (OBJECT(tool)->ops->load != NULL &&
		    object_load(tool, NULL) == -1) {
			text_msg("Error loading", "%s", error_get());
		}
	}
	return (0);
}

int
mapedit_save(void *p, struct netbuf *buf)
{
	int i;

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		struct tool *tool = mapedit.tools[i];

		if (OBJECT(tool)->ops->save != NULL &&
		    object_save(mapedit.tools[i], NULL) == -1) {
			text_msg("Error saving", "%s", error_get());
		}
	}
	return (0);
}

static void
mapedit_win_new_view(int argc, union evarg *argv)
{
	struct mapview *mv, *parent = argv[1].p;
	struct map *m = parent->map;
	struct window *win;
	struct region *reg;

	win = Malloc(sizeof(struct window));
	window_init(win, NULL, WINDOW_SCALE|WINDOW_CENTER, -1, -1,
	    50, 60,
	    50, 60);

	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	mv = mapview_new(reg, m, MAPVIEW_INDEPENDENT, 100, 100);

	win->focus = WIDGET(mv);

	event_new(win, "window-close", window_generic_detach, "%p", win);

	view_attach(win);
	window_show(win);
}

static void
mapedit_win_option(int argc, union evarg *argv)
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
		if (mv->nodeed.win->flags & WINDOW_SHOWN) {
			window_hide(mv->nodeed.win);
		} else {
			window_show(mv->nodeed.win);
		}
		break;
	case MAPEDIT_TOOL_LAYEDIT:
		if (mv->layed.win->flags & WINDOW_SHOWN) {
			window_hide(mv->layed.win);
		} else {
			window_show(mv->layed.win);
		}
		break;
	}
}

struct window *
mapedit_win_new(struct map *m)
{
	struct window *win;
	struct region *reg;
	struct mapview *mv;

	win = Malloc(sizeof(struct window));
	window_init(win, NULL, WINDOW_SCALE|WINDOW_CENTER, -1, -1,
	    60, 70,
	    60, 70);

	mv = Malloc(sizeof(struct mapview));
	mapview_init(mv, m, MAPVIEW_EDIT|MAPVIEW_PROPS|MAPVIEW_INDEPENDENT,
	    100, 100);

	reg = region_new(win, REGION_HALIGN|REGION_CLIPPING, 0, 0, 100, -1);
	region_set_spacing(reg, 1, 1);
	{
		const struct {
			void	(*func)(int argc, union evarg *argv);
			int	icon, toggle, def;
		} fileops[] = {
			{ fileops_revert_map,	MAPEDIT_TOOL_LOAD_MAP,	0, 0 },
			{ fileops_save_map,	MAPEDIT_TOOL_SAVE_MAP,	0, 0 },
			{ fileops_clear_map,	MAPEDIT_TOOL_CLEAR_MAP,	0, 0 },
			{ mapedit_win_new_view,	MAPEDIT_TOOL_NEW_VIEW,	0, 0 },
			{ mapedit_win_option,	MAPEDIT_TOOL_GRID,	1, 0 },
			{ mapedit_win_option,	MAPEDIT_TOOL_PROPS,	1, 1 }
		};
		const int nfileops = sizeof(fileops) / sizeof(fileops[0]);
		struct button *bu;
		struct label *lab;
		int i;

		for (i = 0; i < nfileops; i++) {
			bu = button_new(reg, NULL,
			    SPRITE(&mapedit, fileops[i].icon),
			        fileops[i].toggle ? BUTTON_STICKY : 0,
				-1, -1);
			if (fileops[i].toggle) {
				WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
				event_new(bu, "button-pushed", fileops[i].func,
				    "%p, %i", mv, fileops[i].icon);
			} else {
				WIDGET(bu)->flags |=
				    WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
				event_new(bu, "button-pushed", fileops[i].func,
				    "%p", mv);
			}
			widget_set_bool(bu, "state", fileops[i].def);
		}

		bu = button_new(reg, NULL,	      /* Toggle node edition */
		    SPRITE(&mapedit, MAPEDIT_TOOL_NODEEDIT),
		    BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    mapedit_win_option, "%p, %i", mv, MAPEDIT_TOOL_NODEEDIT);
		mv->nodeed.trigger = bu;
		
		bu = button_new(reg, NULL,	      /* Toggle layer edition */
		    SPRITE(&mapedit, MAPEDIT_TOOL_LAYEDIT),
		    BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    mapedit_win_option, "%p, %i", mv, MAPEDIT_TOOL_LAYEDIT);
		mv->layed.trigger = bu;

		lab = label_polled_new(reg, 60, -1, NULL,
		    " Layer: %d, [%ux%u] at [%d,%d]",
		    &mv->map->cur_layer,
		    &mv->esel.w, &mv->esel.h,
		    &mv->esel.x, &mv->esel.y);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	region_set_spacing(reg, 0, 0);
	{
		region_attach(reg, mv);
		win->focus = WIDGET(mv);
	}
	return (win);
}

