/*	$Csoft: magnifier.c,v 1.25 2003/04/24 07:01:46 vedge Exp $	*/

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

#include "magnifier.h"

#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>

static const struct tool_ops magnifier_ops = {
	{
		tool_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	magnifier_window,
	NULL,			/* cursor */
	NULL,
	magnifier_mouse
};

/* Set the zoom to a specific value in % of the origin size. */
static void
magnifier_zoomN(int argc, union evarg *argv)
{
	struct textbox *tb = argv[0].p;
	struct mapview *mv;

	if ((mv = tool_mapview()) == NULL) {
		text_msg("Error", "%s", error_get());
		return;
	}
	mapview_zoom(mv, textbox_int(tb));
}

/* Set the zoom to 1:1. */
static void
magnifier_zoom100(int argc, union evarg *argv)
{
	struct mapview *mv;
	
	if ((mv = tool_mapview()) == NULL) {
		text_msg("Error", "%s", error_get());
		return;
	}
	mapview_zoom(mv, 100);
}

void
magnifier_init(void *p)
{
	struct magnifier *mag = p;

	tool_init(&mag->tool, "magnifier", &magnifier_ops);
	mag->increment = 30;
}

struct window *
magnifier_window(void *p)
{
	struct magnifier *mag = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-magnifier", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    197, 135,
	    197, 135);
	window_set_caption(win, "Magnifier");

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, -1);
	{
		struct button *button;

		button = button_new(reg, "1:1", NULL, 0, 100, -1);
		event_new(button, "button-pushed", magnifier_zoom100,
		    "%p", mag);
	}

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, -1);
	{
		struct textbox *tbox;

		tbox = textbox_new(reg, "%: ", 0, 100, -1);	/* XXX int */
		event_new(tbox, "textbox-changed", magnifier_zoomN, "%p", mag);
		textbox_printf(tbox, "100");
	}
	return (win);
}

void
magnifier_mouse(void *p, struct mapview *mv, Sint16 xrel, Sint16 yrel,
    Uint8 state)
{
	mapview_zoom(mv, *mv->zoom + xrel);
}

