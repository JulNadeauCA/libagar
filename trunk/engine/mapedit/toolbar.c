/*	$Csoft: config.c,v 1.3 2002/06/12 20:40:08 vedge Exp $	*/

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

static void
tool_push(int argc, union evarg *argv)
{
	struct mapedit *med = argv[1].p;

	dprintf("push\n");

	switch (argv[2].c) {
	case 'n':
		window_show_locked(med->new_map_win);
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

	/*
	 * Create the toolbar.
	 */

	win = window_new("Tool", WINDOW_ABSOLUTE, 32, 32, 63, 128);

	reg = region_new(win, REGION_VALIGN, 0,  0, 50, 100);
	reg->spacing = 1;

	button = button_new(reg, NULL,
	    SPRITE(med, MAPEDIT_FILEOP_NEW_MAP), 0, 0, 0);
	event_new(button, "button-pushed", 0, tool_push, "%p %c", med, 'n');
	win->focus = WIDGET(button);

	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	reg->spacing = 1;

	med->toolbar_win = win;

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

