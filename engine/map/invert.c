/*	$Csoft: invert.c,v 1.2 2005/05/08 02:10:03 vedge Exp $	*/

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

#include <engine/widget/radio.h>

#include "map.h"
#include "mapedit.h"

static void
invert_init(struct tool *t)
{
	tool_push_status(t, _("Specify the tile to invert."));
}

static int
invert_effect(struct tool *t, struct node *n)
{
	struct map *m = t->mv->map;
	struct noderef *nref;
	struct transform *trans;
	
	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;

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
	return (1);
}

const struct tool invert_tool = {
	N_("Color Inversion"),
	N_("Invert the color of a tile."),
	INVERT_TOOL_ICON,
	INVERT_TOOL_ICON,
	invert_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	invert_effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
