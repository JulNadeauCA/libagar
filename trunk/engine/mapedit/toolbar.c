/*	$Csoft: toolbar.c,v 1.11 2002/07/18 11:50:47 vedge Exp $	*/

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
#include <engine/widget/text.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "command.h"
#include "toolbar.h"
#include "fileops.h"
#include "tilestack.h"
#include "tileq.h"
#include "objq.h"

#include "tool/tool.h"
#include "tool/stamp.h"
#include "tool/eraser.h"
#include "tool/magnifier.h"
#include "tool/resize.h"

static void
push(int argc, union evarg *argv)
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
	case MAPEDIT_TOOL_TILEQ:
		window_show(med->tileq_win);
		return;
	case MAPEDIT_TOOL_OBJLIST:
		window_show(med->objlist_win);
		return;
	}

	if (med->curtool != NULL && med->curtool->win != NULL) {
		window_hide(med->curtool->win);
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
	}
	
	if (med->curtool->win != NULL) {
		window_show(med->curtool->win);
	}
	WIDGET_FOCUS(wid);
}

static void
init_tools(struct mapedit *med)
{
	med->tools.stamp = TOOL(stamp_new(med, 0));
	med->tools.eraser = TOOL(eraser_new(med, 0));
	med->tools.magnifier = TOOL(magnifier_new(med, 0));
}

void
mapedit_init_toolbar(struct mapedit *med)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox, *media_tbox, *w_tbox, *h_tbox;
	struct button *button;
	struct tilestack *tstack;
	struct tileq *tqueue;
	struct objq *oqueue;
	
	/* Create the tool objects. */
	init_tools(med);

	/*
	 * Create the toolbar.
	 */
	win = window_new("Tool", WINDOW_SOLID,
	    16, 16,
	    68, 146,
	    68, 146);

	/*
	 * Left side of toolbar.
	 */
	reg = region_new(win, REGION_VALIGN, 0,  0, 50, 100);
	reg->spacing = 1;

	/* New map */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_NEW_MAP), 0, -1, -1);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed", 0, push, "%p %i", med,
	    MAPEDIT_TOOL_NEW_MAP);
	
	/* Object list */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_OBJLIST), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_OBJLIST);
	
	/* Stamp */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_STAMP), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_STAMP);
	
	/* Magnifier */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_MAGNIFIER), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_MAGNIFIER);
	
	/*
	 * Right side of toolbar.
	 */
	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	reg->spacing = 1;
	
	/* Load map */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_LOAD_MAP), 0, -1, -1);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed", 0, push, "%p %i", med,
	    MAPEDIT_TOOL_LOAD_MAP);

	/* Tile list */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_TILEQ), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_TILEQ);
	
	/* Eraser */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_ERASER), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_ERASER);
	
	/* Resize tool */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_RESIZE), 0, -1, -1);
	event_new(button, "button-pushed", 0, push, "%p, %i", med,
	    MAPEDIT_TOOL_RESIZE);
	
	med->toolbar_win = win;

	/*
	 * Related dialog windows.
	 */

	/* Object list window */
	win = window_new("Object", WINDOW_SOLID,
	    83, 16, view->w-96, TILEH+51, TILEW+42, TILEH+51);
	reg = region_new(win, REGION_HALIGN,
	    0, 0, 100, 100);
	oqueue = objq_new(reg, med, 0, 100, 100);
	win->focus = WIDGET(oqueue);
	med->objlist_win = win;

	/* Tile list window */
	win = window_new("Tile", WINDOW_SOLID,
	    view->w-64, 90, 49, view->h-110, TILEW+42, TILEH+51);
	reg = region_new(win, REGION_HALIGN,
	    -6, 0, 100, 100);
	tqueue = tileq_new(reg, med, 0, 100, 100);
	win->focus = WIDGET(tqueue);
	med->tileq_win = win;
	
	/*
	 * Create the `New map' dialog.
	 */
	win = window_new("New map", WINDOW_CENTER,
	     0, 0, 320, 200, 258, 198);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 40);
	name_tbox = textbox_new(reg, "Name: ", 0, 100, 50);
	media_tbox = textbox_new(reg, "Media: ", 0, 100, 50);
	win->focus = WIDGET(name_tbox);

	reg = region_new(win, REGION_HALIGN, 25, 40, 50, 30);
	w_tbox = textbox_new(reg, "W: ", 0, 50, 100);
	textbox_printf(w_tbox, "64");
	h_tbox = textbox_new(reg, "H: ", 0, 50, 100);
	textbox_printf(h_tbox, "64");

	reg = region_new(win, REGION_HALIGN, 2, 70, 98, 25);
	button = button_new(reg, "Create map", NULL, 0, 100, 100);
	event_new(button, "button-pushed", 0, fileops_new_map,
	    "%p, %p, %p, %p, %p",
	    med, name_tbox, media_tbox, w_tbox, h_tbox);
	event_new(name_tbox, "textbox-return", 0, fileops_new_map,
	    "%p, %p, %p, %p, %p",
	    med, name_tbox, media_tbox, w_tbox, h_tbox);

	med->new_map_win = win;
	
	/*
	 * Create the `Load map' dialog.
	 */
	win = window_new("Load map", WINDOW_CENTER,
	    0, 0, 320, 128, 120, 96);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 99);
	/* Map name */
	name_tbox = textbox_new(reg, "Name: ", 0, 100, 50);
	win->focus = WIDGET(name_tbox);
	/* Button */
	button = button_new(reg, "Load map", NULL, 0, 99, 50);
	event_new(button, "button-pushed", 0, fileops_load_map,
	    "%p, %p", med, name_tbox);
	event_new(name_tbox, "textbox-return", 0, fileops_load_map,
	    "%p, %p", med, name_tbox);
	win->focus = WIDGET(name_tbox);

	med->load_map_win = win;
}

