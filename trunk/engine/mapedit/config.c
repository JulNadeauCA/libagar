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
#include <engine/widget/checkbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "command.h"
#include "config.h"

static void
button_pushed(int argc, union evarg *argv)
{
	struct mapedit *med = argv[1].p;

	OBJECT_ASSERT(argv[1].p, "map-editor");

	switch (argv[2].c) {
	case 'c':
		dprintf("close\n");
		window_hide(med->settings_win);
		break;
	case 's':
		object_save(med);
		text_msg(1, TEXT_SLEEP, "Saved map editor settings.\n");
		break;
	}
}

static void
cbox_change(int argc, evargs argv)
{
	struct mapedit *med = argv[0].p;

#if 0
	switch (argv[1].c) {
	case 'p':		/* Props */
		mapedit_editflags(med, MAPEDIT_DRAWPROPS);
		break;
	case 'g':		/* Grid */
		mapedit_editflags(med, MAPEDIT_DRAWGRID);
		break;
	}
#endif
}

struct window *
mapedit_config_win(struct mapedit *med)
{
	struct window *win;
	struct region *body_reg, *buttons_reg;
	struct checkbox *props_cbox, *grid_cbox;
	struct button *close_button, *save_button;
	
	win = window_new("Map edition settings", 0,
	    20,  20,  60,  60);
	body_reg = region_new(win, REGION_VALIGN|REGION_RIGHT,
	     0,   0, 100,  80);
	buttons_reg = region_new(win, REGION_HALIGN|REGION_CENTER,
	     0,  80, 100,  20);
	
	close_button = button_new(buttons_reg, "Close", NULL, 0, 50, 100);
	event_new(close_button, "button-pushed", 0, button_pushed, "%p %c",
	    med, 'c');
	
	save_button = button_new(buttons_reg, "Save", NULL, 0, 50, 100);
	event_new(save_button, "button-pushed", 0, button_pushed, "%p %c",
	    med, 's');

#if 0
	props_cbox = checkbox_new(body_reg, "Show node properties", 50,
	    (med->flags & MAPEDIT_DRAWPROPS) ? CHECKBOX_PRESSED : 0);
	event_new(props_cbox, "checkbox-changed", 0, cbox_change, "%c", 'p');

	grid_cbox = checkbox_new(body_reg, "Show map grid", 50,
	    (med->flags & MAPEDIT_DRAWGRID) ? CHECKBOX_PRESSED : 0);
	event_new(grid_cbox, "checkbox-changed", 0, cbox_change, "%c", 'g');
	win->focus = WIDGET(props_cbox);
#endif

	return (win);
}

