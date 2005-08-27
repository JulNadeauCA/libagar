/*	$Csoft: invert.c,v 1.7 2005/07/30 05:01:34 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/widget/widget.h>
#include <engine/widget/primitive.h>

#include "map.h"
#include "mapedit.h"

static void
init(void *p)
{
	tool_push_status(p, _("Specify element and $(L) to invert."));
}

static int
mousebuttondown(void *p, int x, int y, int btn)
{
	mapmod_begin(TOOL(p)->mv->map);
	return (0);
}

static int
mousebuttonup(void *p, int x, int y, int btn)
{
	struct tool *t = p;
	struct mapview *mv = t->mv;
	struct map *m = mv->map;

	if (m->nmods == 0) {
		mapmod_cancel(m);
	}
	mapmod_end(m);
	return (0);
}

static int
effect(void *p, struct node *n)
{
	struct mapview *mv = TOOL(p)->mv;
	struct map *m = mv->map;
	struct noderef *nref;
	struct transform *trans;
	int nmods = 0;

	mapmod_nodechg(m, mv->cx, mv->cy);

	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;
		
		nmods++;

		TAILQ_FOREACH(trans, &nref->transforms, transforms) {
			if (trans->type == TRANSFORM_INVERT) {
				TAILQ_REMOVE(&nref->transforms, trans,
				    transforms);
				break;
			}
		}
		if (trans != NULL)
			continue;

		if ((trans = transform_new(TRANSFORM_INVERT, 0, NULL))
		    == NULL) {
			text_msg(MSG_ERROR, "%s", error_get());
			continue;
		}
		TAILQ_INSERT_TAIL(&nref->transforms, trans, transforms);
		break;
	}
	return (nmods);
}

static int
cursor(void *p, SDL_Rect *rd)
{
	Uint8 c[4] = { 255, 255, 255, 64 };

	primitives.rect_blended(TOOL(p)->mv, rd->x, rd->y, rd->w, rd->h, 
	    c, ALPHA_OVERLAY);
	return (1);
}

const struct tool_ops invert_ops = {
	"Invert", N_("Invert sprite color"),
	INVERT_TOOL_ICON,
	sizeof(struct tool),
	0,
	init,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	cursor,
	effect,
	
	NULL,			/* mousemotion */
	mousebuttondown,
	mousebuttonup,
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
