/*	$Csoft: mapedit.c,v 1.130 2003/01/19 12:09:40 vedge Exp $	*/

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
	struct mapedit *med = argv[1].p;
	
	switch (argv[2].i) {
	case MAPEDIT_TOOL_NEW_MAP:
		window_show(med->new_map_win);
		return;
	case MAPEDIT_TOOL_LOAD_MAP:
		window_show(med->load_map_win);
		return;
	case MAPEDIT_TOOL_OBJLIST:
		window_show(med->objlist_win);
		return;
	}

	if (med->curtool != NULL && med->curtool->win != NULL) {
		window_hide(med->curtool->win);
		widget_set_bool(med->curtool->button, "value", 0);
	}

	switch (argv[2].i) {
	case MAPEDIT_TOOL_STAMP:
		med->curtool = med->tools.stamp;
		break;
	case MAPEDIT_TOOL_ERASER:
		med->curtool = med->tools.eraser;
		break;
	case MAPEDIT_TOOL_MAGNIFIER:
		med->curtool = med->tools.magnifier;
		break;
	case MAPEDIT_TOOL_RESIZE:
		med->curtool = med->tools.resize;
		break;
	case MAPEDIT_TOOL_PROPEDIT:
		med->curtool = med->tools.propedit;
		break;
	}
	
	if (med->curtool->win != NULL) {
		window_show(med->curtool->win);
	}
	WIDGET_FOCUS(wid);
}



void
mapedit_init(struct mapedit *med, char *name)
{
	const int xdiv = 100, ydiv = 25;
	struct window *win;
	struct region *reg;
	struct button *button;

	object_init(&med->obj, "map-editor", name, "mapedit",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &mapedit_ops);
	med->curtool = NULL;
	med->src_node = NULL;
	med->tools.stamp = TOOL(stamp_new());
	med->tools.eraser = TOOL(eraser_new());
	med->tools.magnifier = TOOL(magnifier_new());
	med->tools.resize = TOOL(resize_new());
	med->tools.propedit = TOOL(propedit_new());
	
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
	
	/* Create the toolbar. */
	win = window_new("mapedit-toolbar", 0, 0, 0, 94, 153, 63, 126);
	window_set_caption(win, "Tools");
	reg = region_new(win, REGION_VALIGN, 0, 0, 50, 100);
	reg->spacing = 1;
	{
		/* New map */
		button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_NEW_MAP), 0, xdiv, ydiv);
		win->focus = WIDGET(button);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p %i", med, MAPEDIT_TOOL_NEW_MAP);
	
		/* Object list */
		button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_OBJLIST), 0, xdiv, ydiv);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_OBJLIST);
	
		/* Stamp */
		med->tools.stamp->button = button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_STAMP), BUTTON_STICKY, xdiv, ydiv);
		WIDGET(button)->flags |= WIDGET_NO_FOCUS;
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_STAMP);
	
		/* Magnifier */
		med->tools.magnifier->button = button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_MAGNIFIER), BUTTON_STICKY,
		    xdiv, ydiv);
		WIDGET(button)->flags |= WIDGET_NO_FOCUS;
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_MAGNIFIER);
	}
	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	reg->spacing = 1;
	{
		/* Load map */
		button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_LOAD_MAP), 0, xdiv, ydiv);
		win->focus = WIDGET(button);
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p %i", med, MAPEDIT_TOOL_LOAD_MAP);

		/* Eraser */
		med->tools.eraser->button = button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_ERASER), BUTTON_STICKY,
		    xdiv, ydiv);
		WIDGET(button)->flags |= WIDGET_NO_FOCUS;
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_ERASER);
	
		/* Resize tool */
		med->tools.resize->button = button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_RESIZE), BUTTON_STICKY,
		    xdiv, ydiv);
		WIDGET(button)->flags |= WIDGET_NO_FOCUS;
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_RESIZE);
	
		/* Property edition tool */
		med->tools.propedit->button = button = button_new(reg, NULL,
		    SPRITE(med, MAPEDIT_TOOL_PROPEDIT), BUTTON_STICKY,
		    xdiv, ydiv);
		WIDGET(button)->flags |= WIDGET_NO_FOCUS;
		event_new(button, "button-pushed", mapedit_select_tool,
		    "%p, %i", med, MAPEDIT_TOOL_PROPEDIT);
	}
	med->toolbar_win = win;

	/* Create the related dialog windows. */
	med->objlist_win = objq_window(med);
	med->new_map_win = fileops_new_map_window(med);
	med->load_map_win = fileops_load_map_window(med);
}

void
mapedit_attached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;

	window_show(med->toolbar_win);
	window_show(med->objlist_win);

	mapedit = med;
}

void
mapedit_detached(int argc, union evarg *argv)
{
	mapedit = NULL;
}

