/*	$Csoft: magnifier.c,v 1.37 2003/09/17 05:31:21 vedge Exp $	*/

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
#include <engine/view.h>
#include <engine/mapedit/mapedit.h>

#include <engine/widget/button.h>
#include <engine/widget/spinbutton.h>

static void magnifier_init(void);
static void magnifier_mouse(struct mapview *, Sint16, Sint16, Uint8);

struct tool magnifier_tool = {
	N_("Magnifier"),
	N_("Zoom in and out specific areas of the map."),
	MAPEDIT_TOOL_MAGNIFIER,
	MAPEDIT_MAGNIFIER_CURSOR,
	magnifier_init,
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* destroy */
	NULL,			/* effect */
	NULL,			/* cursor */
	magnifier_mouse
};

static int zoom_spec = 100;			/* Specific zoom in % */

/* Set the zoom to a specific value in %. */
static void
zoom_specific(int argc, union evarg *argv)
{
	struct mapview *mv;

	if ((mv = tool_mapview()) != NULL)
		mapview_zoom(mv, zoom_spec);
}

/* Set the zoom to 1:1. */
static void
zoom_100(int argc, union evarg *argv)
{
	struct mapview *mv;
	
	if ((mv = tool_mapview()) == NULL) {
		text_msg(MSG_ERROR, _("There is no visible map."));
		return;
	}
	mapview_zoom(mv, 100);
	zoom_spec = 100;
}

static void
magnifier_init(void)
{
	struct window *win;
	struct button *bu;
	struct spinbutton *sbu;

	win = tool_window_new(&magnifier_tool, "mapedit-tool-magnifier");

	bu = button_new(win, _("1:1 zoom"));
	WIDGET(bu)->flags |= WIDGET_WFILL;
	event_new(bu, "button-pushed", zoom_100, NULL);

	sbu = spinbutton_new(win, _("Zoom %%: "));
	widget_bind(sbu, "value", WIDGET_INT, &zoom_spec);
	spinbutton_set_min(sbu, 4);
	spinbutton_set_max(sbu, 600);
	event_new(sbu, "spinbutton-changed", zoom_specific, NULL);
}

static void
magnifier_mouse(struct mapview *mv, Sint16 xrel, Sint16 yrel, Uint8 state)
{
	mapview_zoom(mv, *mv->zoom + xrel);
}

