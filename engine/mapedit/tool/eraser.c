/*	$Csoft: eraser.c,v 1.13 2002/11/22 08:56:53 vedge Exp $	*/

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
#include "eraser.h"

static const struct tool_ops eraser_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	eraser_window,
	eraser_effect,
	NULL			/* cursor */
};

static void	eraser_event(int, union evarg *);

struct eraser *
eraser_new(struct mapedit *med, int flags)
{
	struct eraser *er;

	er= emalloc(sizeof(struct eraser));
	eraser_init(er, med, flags);

	return (er);
}

void
eraser_init(struct eraser *er, struct mapedit *med, int flags)
{
	tool_init(&er->tool, "eraser", med, &eraser_ops);

	er->flags = flags;
	er->mode = 0;
	er->selection.pobj = NULL;
	er->selection.offs = -1;
}

struct window *
eraser_window(void *p)
{
	struct eraser *er = p;
	struct window *win;
	struct region *reg;
	struct radio *rad;
	static char *mode_items[] = {
		"All",
		"Highest",
		"Lowest",
		"Selective",
		NULL
	};

	win = window_new("mapedit-tool-eraser", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y, 120, 120, 120, 120);
	window_set_caption(win, "Eraser");
	reg = region_new(win, 0, 0, 0, 100, 100);
	rad = radio_new(reg, mode_items, 0);
	event_new(rad, "radio-changed", eraser_event, "%p, %c", er, 'm');
	
	win->focus = WIDGET(rad);

	return (win);
}

void
eraser_event(int argc, union evarg *argv)
{
	struct eraser *er = argv[1].p;

	switch (argv[2].c) {
	case 'm':
		er->mode = argv[4].c;
		break;
	}
}

void
eraser_effect(void *p, struct mapview *mv, Uint32 x, Uint32 y)
{
	struct eraser *er = p;
	struct map *m = mv->map;
	struct node *n = &m->map[y][x];
	struct noderef *nref, *nnref;

	switch (er->mode) {
	case ERASER_ALL:
		for (nref = TAILQ_FIRST(&n->nrefsh);
		     nref != TAILQ_END(&n->nrefsh);
		     nref = nnref) {
			nnref = TAILQ_NEXT(nref, nrefs);
			free(nref);
		}
		n->nnrefs = 0;
		TAILQ_INIT(&n->nrefsh);
		break;
	case ERASER_HIGHEST:
		if (!TAILQ_EMPTY(&n->nrefsh)) {
			node_delref(n, TAILQ_LAST(&n->nrefsh, noderefq));
		}
		break;
	case ERASER_LOWEST:
		if (!TAILQ_EMPTY(&n->nrefsh)) {
			node_delref(n, TAILQ_FIRST(&n->nrefsh));
		}
		break;
	case ERASER_SELECTIVE:
		TAILQ_FOREACH(nref, &n->nrefsh, nrefs) {
			if (nref->pobj == er->selection.pobj &&
			    nref->offs == er->selection.offs) {
				node_delref(n, nref);
			}
		}
		break;
	default:
		break;
	}
}

