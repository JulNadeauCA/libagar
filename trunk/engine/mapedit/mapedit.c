/*	$Csoft: mapedit.c,v 1.198 2004/03/25 08:37:22 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/prop.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>

#include "mapedit.h"

const struct object_ops mapedit_ops = {
	NULL,				/* init */
	NULL,				/* reinit */
	mapedit_destroy,
	NULL,				/* load */
	NULL,				/* save */
	NULL				/* edit */
};

const struct object_ops mapedit_pseudo_ops = {
	NULL,			/* init */
	NULL,			/* reinit */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	mapedit_settings	/* edit */
};

struct mapedit mapedit;
int mapedition = 0;			/* Start up in edition mode */

void
mapedit_init(void)
{
	struct window *win, *objedit_win;
	struct box *bo;
	int i;
	
	object_init(&mapedit, "object", "map-editor", &mapedit_ops);
	object_wire_gfx(&mapedit, "/engine/mapedit/mapedit");
	OBJECT(&mapedit)->flags |= (OBJECT_RELOAD_PROPS|OBJECT_STATIC);
	OBJECT(&mapedit)->save_pfx = "/map-editor";

	/* Attach a pseudo-object for dependency keeping purposes. */
	object_init(&mapedit.pseudo, "object", "map-editor",
	    &mapedit_pseudo_ops);
	OBJECT(&mapedit.pseudo)->flags |= (OBJECT_NON_PERSISTENT|OBJECT_STATIC|
	                                   OBJECT_INDESTRUCTIBLE);
	object_attach(world, &mapedit.pseudo);

	/*
	 * Allocate the copy/paste buffer.
	 * Use OBJECT_READONLY to avoid circular reference in case a user
	 * attempts to paste contents of the copy buffer into itself.
	 */
	map_init(&mapedit.copybuf, "copybuf");
	OBJECT(&mapedit.copybuf)->flags |= (OBJECT_NON_PERSISTENT|OBJECT_STATIC|
	                                    OBJECT_INDESTRUCTIBLE|
					    OBJECT_READONLY);
	object_attach(&mapedit.pseudo, &mapedit.copybuf);

	mapedition = 1;

	/* Initialize the default tunables. */
	prop_set_uint32(&mapedit, "default-map-width", 64);
	prop_set_uint32(&mapedit, "default-map-height", 32);
	prop_set_uint32(&mapedit, "default-brush-width", 5);
	prop_set_uint32(&mapedit, "default-brush-height", 5);
	prop_set_bool(&mapedit, "sel-bounded-edition", 0);

	/* Create the object editor window. */
	objedit_init();
	objedit_win = objedit_window();
	window_show(objedit_win);

	/* Create the toolbar window. */
	win = window_new("mapedit-toolbar");
	window_set_caption(win, _("Tools"));
	window_set_closure(win, WINDOW_IGNORE);
	window_set_position(win, WINDOW_UPPER_LEFT, 0);

	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL|BOX_HFILL);
	box_set_spacing(bo, 0);
	{
		struct button *bu;
	
		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_OBJEDITOR));
		event_new(bu, "button-pushed", window_generic_show, "%p",
		    objedit_win);
	}
	window_show(win);
}

void
mapedit_destroy(void *p)
{
	map_destroy(&mapedit.copybuf);
	objedit_destroy();
}

struct window *
mapedit_settings(void *p)
{
	extern int mapview_bg, mapview_bg_moving, mapview_bg_squaresize;
	struct window *win;
	struct checkbox *cb;
	struct spinbutton *sb;
	struct mspinbutton *msb;
	struct box *bo;

	win = window_new(NULL);
	window_set_caption(win, _("Map editor settings"));

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	box_set_spacing(bo, 5);
	{
		cb = checkbox_new(bo, _("Enable background tiling"));
		widget_bind(cb, "state", WIDGET_INT, &mapview_bg);

		cb = checkbox_new(bo, _("Moving tiles"));
		widget_bind(cb, "state", WIDGET_INT, &mapview_bg_moving);

		sb = spinbutton_new(bo, _("Tile size: "));
		widget_bind(sb, "value", WIDGET_INT, &mapview_bg_squaresize);
		spinbutton_set_min(sb, 2);
		spinbutton_set_max(sb, 16384);
	}

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		cb = checkbox_new(bo, _("Selection-bounded edition"));
		widget_bind(cb, "state", WIDGET_PROP, &mapedit,
		    "sel-bounded-edition");
	}

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		msb = mspinbutton_new(bo, "x", _("Default map geometry: "));
		widget_bind(msb, "xvalue", WIDGET_PROP, &mapedit,
		    "default-map-width");
		widget_bind(msb, "yvalue", WIDGET_PROP, &mapedit,
		    "default-map-height");
		mspinbutton_set_min(msb, 1);
		mspinbutton_set_max(msb, MAP_MAX_WIDTH);
		
		msb = mspinbutton_new(bo, "x", _("Default brush geometry: "));
		widget_bind(msb, "xvalue", WIDGET_PROP, &mapedit,
		    "default-brush-width");
		widget_bind(msb, "yvalue", WIDGET_PROP, &mapedit,
		    "default-brush-height");
		mspinbutton_set_min(msb, 1);
		mspinbutton_set_max(msb, MAP_MAX_WIDTH);
	}
	return (win);
}
