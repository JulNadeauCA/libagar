/*	$Csoft: magnifier.c,v 1.44 2004/05/24 03:26:46 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/view.h>
#include <engine/mapedit/mapedit.h>

#include <engine/widget/button.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/checkbox.h>

static void magnifier_init(struct tool *);
static void magnifier_mousebuttondown(struct tool *, int, int, int, int, int);

const struct tool magnifier_tool = {
	N_("Magnifier"),
	N_("Zoom in and out specific areas of the map."),
	MAGNIFIER_TOOL_ICON,
	MAGNIFIER_CURSOR,
	magnifier_init,
	NULL,				/* load */
	NULL,				/* save */
	NULL,				/* destroy */
	NULL,				/* cursor */
	NULL,				/* effect */
	NULL,				/* mousemotion */
	magnifier_mousebuttondown,
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};

static int zoom_spec = 100;

static void
zoom_specific(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	mapview_zoom(mv, zoom_spec);
}

static void
zoom_100(struct tool *t, int state)
{
	mapview_zoom(t->mv, 100);
	zoom_spec = 100;
}

static void
zoom_in(struct tool *t, int state)
{
	struct mapview *mv = t->mv;

	mapview_zoom(mv, *mv->zoom + mv->zoom_inc);
	mv->flags &= ~(MAPVIEW_ZOOMING_OUT);
	mv->flags |= MAPVIEW_ZOOMING_IN;
}

static void
zoom_out(struct tool *t, int state)
{
	struct mapview *mv = t->mv;

	mapview_zoom(mv, *mv->zoom + mv->zoom_inc);
	mv->flags &= ~(MAPVIEW_ZOOMING_IN);
	mv->flags |= MAPVIEW_ZOOMING_OUT;
}

static void
magnifier_init(struct tool *t)
{
	struct window *win;
	struct button *bu;
	struct spinbutton *sbu;
	struct checkbox *cb;
	
	win = tool_window(t, "mapedit-tool-magnifier");

	sbu = spinbutton_new(win, _("Zoom %%: "));
	widget_bind(sbu, "value", WIDGET_INT, &zoom_spec);
	spinbutton_set_min(sbu, 4);
	spinbutton_set_max(sbu, 600);
	event_new(sbu, "spinbutton-changed", zoom_specific, "%p", t->mv);

	tool_push_status(t, _("Click to [zoom-in/center/zoom-out]"));
	tool_bind_key(t, KMOD_NONE, SDLK_0, zoom_100, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_EQUALS, zoom_in, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_MINUS, zoom_out, 0);
}

static void
magnifier_mousebuttondown(struct tool *t, int x, int y, int xoff, int yoff,
    int b)
{
	struct mapview *mv = t->mv;
	int z = *mv->zoom;

	/* XXX logarithmic */
	if (b == 1) {
		z += 10;
	} else if (b == 3) {
		z -= 10;
	}
	mapview_zoom(mv, z);
	mapview_center(mv, x, y);
	*mv->ssx = TILESZ+xoff;
	*mv->ssy = TILESZ+yoff;
}

