/*	$Csoft: shift.c,v 1.20 2003/01/19 12:09:42 vedge Exp $	*/

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
#include <engine/widget/scrollbar.h>
#include <engine/widget/text.h>
#include <engine/widget/tlist.h>
#include <engine/widget/radio.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "shift.h"

static const struct tool_ops shift_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	shift_window,
	NULL,			/* effect */
	NULL			/* cursor */
};

struct shift *
shift_new(void)
{
	struct shift *sh;

	sh = emalloc(sizeof(struct shift));
	shift_init(sh);
	return (sh);
}

void
shift_init(struct shift *sh)
{
	tool_init(&sh->tool, "shift", &shift_ops);
}

struct window *
shift_window(void *p)
{
	struct shift *sh = p;
	struct window *win;
	struct region *reg;
	struct scrollbar *xsb, *ysb;

	win = window_new("mapedit-tool-shift", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    141, 158,
	    165, 177);
	window_set_caption(win, "Shift");

	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	{
		struct radio *rad;
		static const char *modes[] = {
			"Highest",
			"Lowest",
			"Selective",
			"All",
			NULL
		};
		
		rad = radio_new(reg, modes);
		widget_bind(rad, "value", WIDGET_INT, NULL, &sh->mode);
	}
	return (win);
}

void
shift_mouse(void *p, struct mapview *mv, Sint16 relx, Sint16 rely)
{
	struct shift *sh = p;
	struct tlist_item *it;
	struct node *node = mv->cur_node;
	struct noderef *nref;

	switch (sh->mode) {
	case SHIFT_HIGHEST:
		if (!TAILQ_EMPTY(&node->nrefs)) {
			nref = TAILQ_LAST(&node->nrefs, noderefq);
			nref->xcenter += relx;
			nref->ycenter += rely;
		}
		break;
	case SHIFT_LOWEST:
		if (!TAILQ_EMPTY(&node->nrefs)) {
			nref = TAILQ_FIRST(&node->nrefs);
			nref->xcenter += relx;
			nref->ycenter += rely;
		}
		break;
	case SHIFT_SELECTIVE:
		TAILQ_FOREACH(it, &mv->node_tlist->items, items) {
			if (it->selected) {
				nref = it->p1;
				nref->xcenter += relx;
				nref->ycenter += rely;
			}
		}
		break;
	case SHIFT_ALL:
		TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
			nref->xcenter += relx;
			nref->ycenter += rely;
		}
		break;
	}
}

