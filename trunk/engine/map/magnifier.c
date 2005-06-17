/*	$Csoft: magnifier.c,v 1.5 2005/06/16 16:04:17 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef MAP

#include <engine/view.h>

#include <engine/widget/button.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/checkbox.h>

#include "map.h"
#include "mapedit.h"

int magnifier_zoom_toval = 100;
int magnifier_zoom_inc = 8;

static void
update_status(struct mapview *mv)
{
	mapview_status(mv, _("%d%% zoom"), MV_ZOOM(mv));
}

static void
zoom_to_value(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	mapview_set_scale(mv, magnifier_zoom_toval, 1);
	update_status(mv);
}

static int
zoom1(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;

	mapview_set_scale(mv, 100, 1);
	update_status(mv);
	return (1);
}

static int
zoom_in(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;

	mapview_set_scale(mv, MV_ZOOM(mv) + magnifier_zoom_inc, 1);
	update_status(mv);
	return (1);
}

static int
zoom_out(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;

	mapview_set_scale(mv, MV_ZOOM(mv) - magnifier_zoom_inc, 1);
	update_status(mv);
	return (1);
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
	widget_bind(sbu, "value", WIDGET_INT, &magnifier_zoom_toval);
	spinbutton_set_min(sbu, 4);
	spinbutton_set_max(sbu, 600);
	spinbutton_set_increment(sbu, magnifier_zoom_inc);
	event_new(sbu, "spinbutton-changed", zoom_to_value, "%p", t->mv);

	sbu = spinbutton_new(win, _("Zoom increment: "));
	widget_bind(sbu, "value", WIDGET_INT, &magnifier_zoom_inc);
	spinbutton_set_min(sbu, 1);
	
	tool_bind_key(t, KMOD_NONE, SDLK_0, zoom1, NULL);
	tool_bind_key(t, KMOD_NONE, SDLK_1, zoom1, NULL);
	tool_bind_key(t, KMOD_NONE, SDLK_EQUALS, zoom_in, NULL);
	tool_bind_key(t, KMOD_NONE, SDLK_MINUS, zoom_out, NULL);
}

const struct tool magnifier_tool = {
	N_("Magnifier"),
	N_("Zoom to specific areas on the map."),
	MAGNIFIER_TOOL_ICON, MAGNIFIER_CURSORBMP,
	TOOL_HIDDEN,
	magnifier_init,
	NULL,				/* load */
	NULL,				/* save */
	NULL,				/* destroy */
	NULL,				/* cursor */
	NULL,				/* effect */
	NULL,				/* mousemotion */
	NULL,				/* mousebuttondown */
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};

#endif /* MAP */
