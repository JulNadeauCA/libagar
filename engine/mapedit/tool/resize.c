/*	$Csoft: resize.c,v 1.16 2003/02/02 21:14:02 vedge Exp $	*/

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

#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "resize.h"

static const struct tool_ops resize_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	resize_window,
	NULL,			/* cursor */
	NULL,			/* effect */
	resize_mouse
};

static void	resize_effect(int, union evarg *);

void
resize_init(void *p)
{
	struct resize *res = p;

	tool_init(&res->tool, "resize", &resize_ops);

	res->mode = RESIZE_GROW;
	res->cx = -1;
	res->cy = -1;
}

struct window *
resize_window(void *p)
{
	struct resize *res = p;
	struct window *win;
	struct region *reg;
	struct button *button;
	struct textbox *tbox_w, *tbox_h;

	win = window_new("window-tool-resize", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    164, 78,
	    164, 78);
	window_set_caption(win, "Resize map");

	/* Scale textboxes */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
	tbox_w = textbox_new(reg, "W: ", 0, 50, -1);	/* XXX int */
	win->focus = WIDGET(tbox_w);
	tbox_h = textbox_new(reg, "H: ", 0, 50, -1);	/* XXX int */
	textbox_printf(tbox_w, "0");
	textbox_printf(tbox_h, "0");

	/* Resize button */
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	button = button_new(reg, "Resize", NULL, 0, 100, 100);
	event_new(button, "button-pushed",
	    resize_effect, "%p, %p, %p", res, tbox_w, tbox_h);

	return (win);
}

static void
resize_effect(int argc, union evarg *argv)
{
	struct resize *res = argv[1].p;
	struct textbox *tbox_w = argv[2].p, *tbox_h = argv[3].p;
	struct mapview *mv;
	struct map *m;
	Uint32 w, h;

	mv = tool_mapview();
	if (mv == NULL)
		return;			/* No selection */
	m = mv->map;

	w = (Uint32)textbox_int(tbox_w);
	h = (Uint32)textbox_int(tbox_h);

	switch (res->mode) {
	case RESIZE_GROW:
		map_grow(m, w, h);
		break;
	case RESIZE_SHRINK:
		map_shrink(m, w, h);
		break;
	}
}

void
resize_mouse(void *p, struct mapview *mv, Sint16 xrel, Sint16 yrel, Uint8 state)
{
	struct resize *res = p;
	struct map *m = mv->map;

	if ((state & SDL_BUTTON(1)) == 0) {
		return;
	}

	if (xrel > 0) {
		map_grow(m, m->mapw + xrel, m->maph);
	} else if (xrel < 0 && ((int)m->mapw + xrel) > 2) {
		map_shrink(m, m->mapw + xrel, m->maph);
	}
		
	if (yrel > 0) {
		map_grow(m, m->mapw, m->maph + yrel);
	} else if (yrel < 0 && ((int)m->maph + yrel) > 2) {
		map_shrink(m, m->mapw, m->maph + yrel);
	}
}
