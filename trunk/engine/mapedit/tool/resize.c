/*	$Csoft: resize.c,v 1.10 2002/11/15 04:18:32 vedge Exp $	*/

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

#include <engine/engine.h>

#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>

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
	NULL,			/* effect */
	NULL			/* cursor */
};

static void	resize_do(int, union evarg *);

struct resize *
resize_new(struct mapedit *med, int flags)
{
	struct resize *resize;

	resize = emalloc(sizeof(struct resize));
	resize_init(resize, med, flags);

	return (resize);
}

void
resize_init(struct resize *res, struct mapedit *med, int flags)
{
	tool_init(&res->tool, "resize", med, &resize_ops);

	res->flags = flags;
	res->mode = 0;
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
	    174, 110,
	    174, 110);
	window_set_caption(win, "Resize map");

	/* Scale textboxes */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 40);
	tbox_w = textbox_new(reg, "W: ", 0, 50, 100);	/* XXX int */
	win->focus = WIDGET(tbox_w);
	tbox_h = textbox_new(reg, "H: ", 0, 50, 100);	/* XXX int */
	textbox_printf(tbox_w, "0");
	textbox_printf(tbox_h, "0");

	/* Resize button */
	reg = region_new(win, REGION_HALIGN, 0, 40, 100, 60);
	button = button_new(reg, "Resize", NULL, 0, 100, 100);
	event_new(button, "button-pushed",
	    resize_do, "%p, %p, %p", res, tbox_w, tbox_h);

	return (win);
}

static void
resize_do(int argc, union evarg *argv)
{
	struct resize *res = argv[1].p;
	struct textbox *tbox_w = argv[2].p, *tbox_h = argv[3].p;
	struct mapview *mv = tool_mapview();
	struct map *m = mv->map;
	Uint32 w, h;

	mv = tool_mapview();
	if (mv == NULL) {
		warning("no view\n");
		return;
	}
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

