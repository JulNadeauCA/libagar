/*	$Csoft: mapedit.c,v 1.133 2003/01/24 08:27:02 vedge Exp $	*/

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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "fileops.h"
#include "objq.h"

#include "tool/tool.h"
#include "tool/stamp.h"
#include "tool/eraser.h"
#include "tool/magnifier.h"
#include "tool/resize.h"
#include "tool/propedit.h"
#include "tool/select.h"
#include "tool/shift.h"

static const struct version mapedit_ver = {
	"agar map editor",
	1, 0
};

static const struct object_ops mapedit_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

struct mapedit *mapedit = NULL;		/* Map editor */

static void
mapedit_select_tool(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct tool *newtool = argv[1].p;

	if (mapedit->curtool == newtool) {
		if (newtool->win != NULL) {
			window_hide(newtool->win);
		}
		mapedit->curtool = NULL;
		return;
	}

	if (mapedit->curtool != NULL) {
		widget_set_bool(mapedit->curtool->button, "state", 0);
		if (mapedit->curtool->win != NULL &&
		   (mapedit->curtool->win->flags & WINDOW_SHOWN)) {
			window_hide(mapedit->curtool->win);
		}
	}

	mapedit->curtool = newtool;

	if (newtool->win != NULL) {
		window_show(newtool->win);
	}
}

void
mapedit_init(struct mapedit *med, char *name)
{
	const int xdiv = 100, ydiv = 20;
	struct window *win;
	struct region *reg;
	struct button *button;

	object_init(&med->obj, "map-editor", name, "mapedit",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &mapedit_ops);
	med->curtool = NULL;
	med->src_node = NULL;
	
	prop_set_int(med, "zoom-minimum", 4);
	prop_set_int(med, "zoom-maximum", 400);
	prop_set_int(med, "zoom-increment", 2);
	prop_set_int(med, "zoom-speed", 60);
	prop_set_int(med, "tilemap-item-size", 16);
	prop_set_bool(med, "tilemap-scroll-x", 0);
	prop_set_bool(med, "tilemap-scroll-y", 1);
	prop_set_bool(med, "tilemap-bg-moving", 1);
	prop_set_int(med, "tilemap-bg-square-size", 16);
	
	event_new(med, "attached", mapedit_attached, NULL);
	event_new(med, "detached", mapedit_detached, NULL);

	med->tools.stamp = TOOL(stamp_new());
	med->tools.eraser = TOOL(eraser_new());
	med->tools.magnifier = TOOL(magnifier_new());
	med->tools.resize = TOOL(resize_new());
	med->tools.propedit = TOOL(propedit_new());
	med->tools.select = TOOL(select_new());
	med->tools.shift = TOOL(shift_new());

	/* Create the dialogs. */
	med->win.objlist = objq_window(med);
	med->win.new_map = fileops_new_map_window(med);
	med->win.load_map = fileops_load_map_window(med);
	
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
	
		button = med->tools.stamp->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_STAMP),
		    BUTTON_NOFOCUS|BUTTON_STICKY, xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.stamp);

		button = med->tools.magnifier->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_MAGNIFIER),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.magnifier);
		
		button = med->tools.select->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_SELECT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.select);
	}

	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	region_set_spacing(reg, 0, 0);
	{
		button = button_new(reg, NULL,			/* Load map */
		    SPRITE(med, MAPEDIT_TOOL_LOAD_MAP), 0, xdiv, ydiv);
		win->focus = WIDGET(button);
		event_new(button, "button-pushed", window_generic_show,
		    "%p", med->win.load_map);

		button = med->tools.eraser->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_ERASER),
		    BUTTON_STICKY|BUTTON_NOFOCUS,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.eraser);

		button = med->tools.resize->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_RESIZE),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.resize);

		button = med->tools.propedit->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_PROPEDIT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.propedit);

		button = med->tools.shift->button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_SHIFT),
		    BUTTON_NOFOCUS|BUTTON_STICKY,
		    xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p", med->tools.shift);
	}
}

void
mapedit_attached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;

	window_show(med->win.toolbar);
	window_show(med->win.objlist);

	mapedit = med;
}

void
mapedit_detached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;

	window_hide(med->win.toolbar);
	window_hide(med->win.objlist);

	mapedit = NULL;
}

