/*	$Csoft: toolbar.c,v 1.1 2002/06/22 20:49:17 vedge Exp $	*/

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

static void
push(int argc, union evarg *argv)
{
	struct mapedit *med = argv[1].p;

	switch (argv[2].c) {
	case 'n':
		window_show_locked(med->new_map_win);
		break;
	case 'o':
		window_show_locked(med->objlist_win);
		break;
	case 'q':
		window_show_locked(med->tileq_win);
		break;
	case 's':
		window_show_locked(med->tilestack_win);
		break;
	}
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

	/*
	 * Create the toolbar.
	 */
	win = window_new("Tool", WINDOW_ABSOLUTE, 16, 16, 63, 128);

	reg = region_new(win, REGION_VALIGN, 0,  0, 50, 100);
	reg->spacing = 1;

	/* New map */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_NEW_MAP), 0, 0, 0);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed", 0, push, "%p %c", med, 'n');
	
	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	reg->spacing = 1;

	/* Object list */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_OBJLIST), 0, 0, 0);
	event_new(button, "button-pushed", 0, push, "%p %c", med, 'o');

	/* Tile list */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_TILEQ), 0, 0, 0);
	event_new(button, "button-pushed", 0, push, "%p %c", med, 'q');

	/* Tile stack */
	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_TOOL_TILESTACK), 0, 0, 0);
	event_new(button, "button-pushed", 0, push, "%p %c", med, 's');

	med->toolbar_win = win;

	/* Object list window */
	win = window_new("Object", WINDOW_ABSOLUTE|WINDOW_SOLID,
	    80, 16, view->w - 96, 74);
	reg = region_new(win, REGION_HALIGN,
	    0, 0, 100, 100);
	oqueue = objq_new(reg, med, 0, 100, 100);
	win->focus = WIDGET(oqueue);
	med->objlist_win = win;

	/* Tile list window */
	win = window_new("Tile", WINDOW_ABSOLUTE|WINDOW_SOLID,
	    view->w - 80, 80, 48, view->h - 110);
	reg = region_new(win, REGION_HALIGN,
	    0, 0, 100, 100);
	tqueue = tileq_new(reg, med, 0, 100, 100);
	win->focus = WIDGET(tqueue);
	med->tileq_win = win;

	/* Tile stack window */
	win = window_new("Stack", WINDOW_ABSOLUTE|WINDOW_SOLID,
	    80, 80, 64, view->h - 96);
	reg = region_new(win, REGION_HALIGN,
	    0, 0, 100, 100);
	tstack = tilestack_new(reg, 0, 100, 100);
	med->tilestack_win = win;

	/*
	 * Create the `New map' dialog.
	 */
	win = window_new("New map", 0, 20, 20, 50, 40);

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

	med->new_map_win = win;
}

