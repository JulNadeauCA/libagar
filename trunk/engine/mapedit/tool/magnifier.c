/*	$Csoft: magnifier.c,v 1.8 2002/09/06 01:26:43 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/version.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/button.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "magnifier.h"

static const struct tool_ops magnifier_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	magnifier_window,
	magnifier_effect,
	NULL			/* cursor */
};

static void	magnifier_event(int, union evarg *);

struct magnifier *
magnifier_new(struct mapedit *med, int flags)
{
	struct magnifier *magnifier;

	magnifier = emalloc(sizeof(struct magnifier));
	magnifier_init(magnifier, med, flags);

	return (magnifier);
}

void
magnifier_init(struct magnifier *mag, struct mapedit *med, int flags)
{
	tool_init(&mag->tool, "magnifier", med, &magnifier_ops);

	mag->flags = flags;
	mag->mode = 0;
}

struct window *
magnifier_window(void *p)
{
	struct magnifier *mag = p;
	struct window *win;
	struct region *reg;
	struct radio *rad;
	struct button *button;
	struct textbox *tbox;
	static char *mode_items[] = {
		"Zoom in",
		"Zoom out",
		"Center",
		NULL
	};

	win = window_new("mapedit-tool-magnifier", "Magnifier", WINDOW_SOLID,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    109, 171,
	    109, 171);
	
	/* Mode */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 50);
	rad = radio_new(reg, mode_items, 0);
	event_new(rad, "radio-changed", 0, magnifier_event,
	    "%p, %c", mag, 'm');
	
	/* Original size button */
	reg = region_new(win, REGION_VALIGN, 0, 50, 97, 20);
	button = button_new(reg, "0:0", NULL, 0, 100, 100);
	event_new(button, "button-pushed", 0, magnifier_event,
	    "%p, %c", mag, 'o');

	/* Scale textbox */
	reg = region_new(win, REGION_VALIGN, 0, 70, 97, 30);
	tbox = textbox_new(reg, "%: ", 0, 100, 100);	/* XXX int */
	event_new(tbox, "textbox-changed", 0, magnifier_event,
	    "%p, %c", mag, 's');
	textbox_printf(tbox, "100");

	win->focus = WIDGET(rad);

	return (win);
}

static void
magnifier_event(int argc, union evarg *argv)
{
	struct magnifier *mag = argv[1].p;
	struct mapview *mv;

	OBJECT_ASSERT(mag, "tool");

	switch (argv[2].c) {
	case 'm':
		mag->mode = argv[4].i;
		break;
	case 'o':
		mv = tool_mapview();
		if (mv != NULL) {
			mapview_zoom(mv, 100);
		}
		break;
	case 's':
		mv = tool_mapview();
		if (mv != NULL) {
			struct textbox *tbox = argv[0].p;
			int fac;

			fac = textbox_int(tbox);
			if (fac < -32767) {
				fac = -32767;
			} else if (fac > 32767) {
				fac = 32767;
			}
			mapview_zoom(mv, fac);
		}
	}
}

void
magnifier_effect(void *p, struct mapview *mv, Uint32 x, Uint32 y)
{
	struct magnifier *mag = p;

	switch (mag->mode) {
	case MAGNIFIER_ZOOM_IN:
		mapview_zoom(mv, mv->zoom + 10);
		mapview_center(mv, x, y);
		break;
	case MAGNIFIER_ZOOM_OUT:
		mapview_zoom(mv, mv->zoom - 10);
		mapview_center(mv, x, y);
		break;
	case MAGNIFIER_CENTER:
		mapview_center(mv, x, y);
		break;
	}
}

