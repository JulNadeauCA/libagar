/*	$Csoft: stamp.c,v 1.4 2002/07/08 08:39:44 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/version.h>

#include <engine/widget/window.h>
#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/label.h>
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
	stamp_effect
};

static void	stamp_event(int, union evarg *);

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
	struct radio *mode_rad;
	static const char *mode_items[] = {
		"Replace",
		"Insert highest",
		"Insert lowest",
		NULL
	};

	win = window_new("Stamp", WINDOW_SOLID,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y, 145, 96, 145, 96);
	
	/* Mode */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	mode_rad = radio_new(reg, mode_items, 0, 0);
	event_new(mode_rad, "radio-changed", 0, stamp_event, "%p, %c", st, 'm');
	win->focus = WIDGET(mode_rad);

	return (win);
}

static void
stamp_event(int argc, union evarg *argv)
{
	struct stamp *st = argv[1].p;

	OBJECT_ASSERT(st, "tool");

	switch (argv[2].c) {
	case 'm':
		st->mode = argv[4].i;
		break;
	}
}

void
stamp_effect(void *p, struct mapview *mv, Uint32 x, Uint32 y)
{
	struct stamp *st = p;
	struct map *m = mv->map;
	struct mapedit *med = TOOL(st)->med;
	struct node *n = &m->map[y][x];
	struct noderef *nref, *nnref;
	struct editref *eref;

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
	}

	SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs, med->curoffs);

	switch (st->mode) {
	case STAMP_INSERT_LOWEST:
		break;
	case STAMP_INSERT_HIGHEST:
	case STAMP_REPLACE:
		switch (eref->type) {
		case EDITREF_SPRITE:
			node_addref(n, med->curobj->pobj, eref->spritei,
			    MAPREF_SPRITE|MAPREF_SAVE);
			break;
		case EDITREF_ANIM:
			node_addref(n, med->curobj->pobj, eref->animi,
			    MAPREF_ANIM|MAPREF_SAVE|MAPREF_ANIM_DELTA);
			break;
		}
		break;
	}

	n->flags = med->curflags &= ~(NODE_ORIGIN|NODE_ANIM);
}

