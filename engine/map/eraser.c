/*	$Csoft: eraser.c,v 1.1 2005/04/14 06:19:40 vedge Exp $	*/

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

#include "map.h"
#include "mapedit.h"

static void eraser_init(struct tool *);
static void eraser_effect(struct tool *, struct node *);

const struct tool eraser_tool = {
	N_("Eraser"),
	N_("Remove the highest node reference."),
	ERASER_TOOL_ICON,
	ERASER_CURSORBMP,
	eraser_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	eraser_effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

static void
eraser_init(struct tool *t)
{
	tool_push_status(t, _("Specify the tile to delete."));
}

static void
eraser_effect(struct tool *t, struct node *n)
{
	struct map *m = t->mv->map;
	struct noderef *nref;
	
	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;

		TAILQ_REMOVE(&n->nrefs, nref, nrefs);
		noderef_destroy(m, nref);
		Free(nref, M_MAP_NODEREF);
		break;
	}
}

#endif /* MAP */
