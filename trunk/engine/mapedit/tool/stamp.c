/*	$Csoft: stamp.c,v 1.14 2002/11/22 05:41:42 vedge Exp $	*/

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
#include <engine/widget/radio.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "stamp.h"

static const struct tool_ops stamp_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	stamp_window,
	stamp_effect,
	NULL			/* cursor */
};

static void	stamp_mode_changed(int, union evarg *);

struct stamp *
stamp_new(struct mapedit *med, int flags)
{
	struct stamp *stamp;

	stamp = emalloc(sizeof(struct stamp));
	stamp_init(stamp, med, flags);

	return (stamp);
}

void
stamp_init(struct stamp *stamp, struct mapedit *med, int flags)
{
	tool_init(&stamp->tool, "stamp", med, &stamp_ops);

	stamp->flags = flags;
	stamp->mode = 0;
}

struct window *
stamp_window(void *p)
{
	struct stamp *st = p;
	struct window *win;
	struct region *reg;
	struct radio *rad;
	static char *mode_items[] = {
		"Replace",
		"Insert highest",
		"Insert lowest",
		NULL
	};

	win = window_new("mapedit-tool-stamp", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y, 156, 101, 156, 101);
	window_set_caption(win, "Stamp");

	/* Mode */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	rad = radio_new(reg, mode_items, 0);
	event_new(rad, "radio-changed",
	    stamp_mode_changed, "%p, %c", st, 'm');
	win->focus = WIDGET(rad);

	return (win);
}

static void
stamp_mode_changed(int argc, union evarg *argv)
{
	struct stamp *st = argv[1].p;

	st->mode = argv[4].i;
}

void
stamp_effect(void *p, struct mapview *mv, Uint32 x, Uint32 y)
{
	struct stamp *st = p;
	struct map *m = mv->map;
	struct mapedit *med = TOOL(st)->med;
	struct node *n = &m->map[y][x];
	struct noderef *nref, *nnref;

	switch (st->mode) {
	case STAMP_REPLACE:
		for (nref = TAILQ_FIRST(&n->nrefsh);
		     nref != TAILQ_END(&n->nrefsh);
		     nref = nnref) {
			nnref = TAILQ_NEXT(nref, nrefs);
			free(nref);
		}
		n->nnrefs = 0;
		TAILQ_INIT(&n->nrefsh);
		break;
	default:
		break;
	}

	switch (st->mode) {
	case STAMP_INSERT_LOWEST:
		break;
	case STAMP_INSERT_HIGHEST:
	case STAMP_REPLACE:
		if (med->ref.flags & MAPREF_SPRITE) {
			node_addref(n, med->ref.obj, med->ref.offs,
			    med->ref.flags);
		} else if (med->ref.flags & MAPREF_ANIM) {
			node_addref(n, med->ref.obj, med->ref.offs,
			    med->ref.flags);
		} else {
			fatal("unknown ref type\n");
		}
		break;
	}

	n->flags = med->node.flags &= ~(NODE_ORIGIN|NODE_ANIM);
}

