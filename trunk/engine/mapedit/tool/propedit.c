/*	$Csoft: propedit.c,v 1.11 2003/01/01 05:18:38 vedge Exp $	*/

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
	propedit_effect,
	NULL			/* cursor */
};

static void	propedit_event(int, union evarg *);

struct propedit *
propedit_new(void)
{
	struct propedit *pe;

	pe = emalloc(sizeof(struct propedit));
	propedit_init(pe);
	return (pe);
}

void
propedit_init(struct propedit *pe)
{
	tool_init(&pe->tool, "propedit", &propedit_ops);

	pe->mode = PROPEDIT_CLEAR;
	pe->nodeflags = 0;
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
	    TOOL_DIALOG_X, TOOL_DIALOG_Y, 149, 198, 149, 198);
	window_set_caption(win, "Node props");

	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 40);

	/* Mode */
	{
		static char *items[] = {
			"Clear",
			"Set",
			"Unset",
			NULL
		};

		rad = radio_new(reg, items, 0);
		event_new(rad, "radio-changed",
		    propedit_event, "%p, %i", pe, -1);
	}

	/* Block/walk/climb */
	{
		static char *items[] = {
			"Block",
			"Walk",
			"Climb",
			NULL
		};

		rad = radio_new(reg, items, 0);
		event_new(rad, "radio-changed",
		    propedit_event, "%p, %i", pe, 0);
	}

	reg = region_new(win, REGION_VALIGN, 0, 40, 100, 60);
	
	/* Node flags */
	{
		const struct {
			int	 flag;
			char	*name;
		} props[] = {
			{ NODE_ORIGIN,	"Origin" },
			{ NODE_BIO,	"Bio"	 },
			{ NODE_REGEN,	"Regen"	 },
			{ NODE_SLOW,	"Slow"	 },
			{ NODE_HASTE,	"Haste"	 }
		};
		const int nprops = sizeof(props) / sizeof(props[0]);
		int i;

		for (i = 0; i < nprops; i++) {
			cbox = checkbox_new(reg, 15, props[i].name);
			event_new(cbox, "checkbox-changed",
			    propedit_event, "%p, %i", pe, props[i].flag);
		}
	}

	win->focus = WIDGET(rad);
	return (win);
}

void
propedit_event(int argc, union evarg *argv)
{
	struct propedit *pe = argv[1].p;
	struct radio *rad;
	int value = argv[2].i;

	switch (value) {
	case -1:					/* Mode */
		{
			struct radio *rad = argv[0].p;

			switch (rad->selitem) {
			case 0:
				pe->mode = PROPEDIT_CLEAR;
				break;
			case 1:
				pe->mode = PROPEDIT_SET;
				break;
			case 2:
				pe->mode = PROPEDIT_UNSET;
				break;
			}
		}
		break;
	case 0:						/* Block */
		{
			struct radio *rad = argv[0].p;
		
			pe->nodeflags &= ~(NODE_BLOCK|NODE_WALK|NODE_CLIMB);

			switch (rad->selitem) {
			case 0:
				pe->nodeflags |= NODE_BLOCK;
				break;
			case 1:
				pe->nodeflags |= NODE_WALK;
				break;
			case 2:
				pe->nodeflags |= NODE_CLIMB;
				break;
			}
		}
		break;
	default:					/* Flag */
		{
			struct checkbox *cbox = argv[0].p;
			int state = argv[1].i;
			
			if (state) {
				pe->nodeflags |= value;
			} else {
				pe->nodeflags &= ~(value);
			}
		}
		break;
	}
}

void
propedit_effect(void *p, struct mapview *mv, Uint32 x, Uint32 y)
{
	struct propedit *pe = p;
	struct map *m = mv->map;
	struct node *n = &m->map[y][x];

	if (pe->nodeflags & NODE_ORIGIN) {
		struct node *on;

		on = &m->map[m->defy][m->defx];
		on->flags &= ~(NODE_ORIGIN);
		n->flags |= NODE_ORIGIN;
		m->defx = x;
		m->defy = y;
		return;
	}

	switch (pe->mode) {
	case PROPEDIT_CLEAR:
		n->flags = pe->nodeflags;
		break;
	case PROPEDIT_SET:
		n->flags |= pe->nodeflags;
		break;
	case PROPEDIT_UNSET:
		n->flags &= ~(pe->nodeflags);
		break;
	}
}

