/*	$Csoft: propedit.c,v 1.21 2003/02/22 11:47:51 vedge Exp $	*/

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
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "propedit.h"

static const struct tool_ops propedit_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	propedit_window,
	NULL,			/* cursor */
	propedit_effect,
	NULL			/* mouse */
};

static void	propedit_event(int, union evarg *);

void
propedit_init(void *p)
{
	struct propedit *pe = p;

	tool_init(&pe->tool, "propedit", &propedit_ops);

	pe->mode = PROPEDIT_CLEAR;
	pe->node_mask = 0;
	pe->node_mode = 0;
}

static void
propedit_set_node_mode(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	struct propedit *pe = argv[1].p;
	int index = argv[2].i;
	const Uint32 modes[] = {
		0,
		NODE_WALK,
		NODE_CLIMB
	};

	pe->node_mode = modes[index];
}

static void
propedit_set_node_flags(int argc, union evarg *argv)
{
	struct checkbox *cbox = argv[0].p;
	struct propedit *pe = argv[1].p;
	int flag = argv[2].i;
	int state = argv[3].i;

	if (state) {
		pe->node_mask |= flag;
	} else {
		pe->node_mask &= ~(flag);
	}
}

struct window *
propedit_window(void *p)
{
	struct propedit *pe = p;
	struct window *win;
	struct region *reg;
	struct radio *rad;
	struct checkbox *cbox;

	win = window_new("mapedit-tool-propedit", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    140, 358,
	    140, 358);
	window_set_caption(win, "Node props");

	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
	{
		static const char *modes[] = {
			"Clear",
			"Set",
			"Unset",
			NULL
		};
		static const char *node_modes[] = {
			"Block",
			"Walk",
			"Climb",
			NULL
		};

		rad = radio_new(reg, modes);
		widget_bind(rad, "value", WIDGET_INT, NULL, &pe->mode);

		rad = radio_new(reg, node_modes);
		event_new(rad, "radio-changed",
		    propedit_set_node_mode, "%p", pe);
	}

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		const struct {
			Uint32	flag;
			char	*name;
		} props[] = {
			{ NODE_ORIGIN,	"Origin" },
			{ NODE_BIO,	"Bio"	 },
			{ NODE_REGEN,	"Regen"	 },
			{ NODE_SLOW,	"Slow"	 },
			{ NODE_HASTE,	"Haste"	 },
			{ NODE_EDGE_N,	"Edge-N" },
			{ NODE_EDGE_S,	"Edge-S" },
			{ NODE_EDGE_W,	"Edge-W" },
			{ NODE_EDGE_E,	"Edge-E" },
			{ NODE_EDGE_NW,	"Edge-NW" },
			{ NODE_EDGE_NE,	"Edge-NE" },
			{ NODE_EDGE_SW,	"Edge-SW" },
			{ NODE_EDGE_SE,	"Edge-SE" }
		};
		const int nprops = sizeof(props) / sizeof(props[0]);
		int i;

		for (i = 0; i < nprops; i++) {
			cbox = checkbox_new(reg, -1, props[i].name);
			event_new(cbox, "checkbox-changed",
			    propedit_set_node_flags, "%p, %i",
			    pe, props[i].flag);
		}
	}

	win->focus = WIDGET(rad);
	return (win);
}

void
propedit_effect(void *p, struct mapview *mv, struct node *node)
{
	struct propedit *pe = p;
	struct map *m = mv->map;

	if (pe->node_mask & NODE_ORIGIN) {
		struct node *oldnode;

		m->defx = mv->cx;
		m->defy = mv->cy;
		return;
	}

	switch (pe->mode) {
	case PROPEDIT_CLEAR:
		node->flags = pe->node_mask;
		break;
	case PROPEDIT_SET:
		node->flags |= pe->node_mask;
		break;
	case PROPEDIT_UNSET:
		node->flags &= ~(pe->node_mask);
		break;
	}
	
	node->flags &= ~(NODE_WALK|NODE_CLIMB);
	if (pe->node_mode == 0) {
		node->flags &= ~(NODE_WALK);
	} else {
		node->flags |= pe->node_mode;
	}
}

