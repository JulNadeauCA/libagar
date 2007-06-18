/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "map.h"
#include "mapedit.h"

static void
init(void *p)
{
	MAP_ToolPushStatus(p, _("Specify element and $(L) to invert."));
}

static int
mousebuttondown(void *p, int x, int y, int btn)
{
	MAP_ModBegin(TOOL(p)->mv->map);
	return (0);
}

static int
mousebuttonup(void *p, int x, int y, int btn)
{
	MAP_Tool *t = p;
	MAP_View *mv = t->mv;
	MAP *m = mv->map;

	if (m->nmods == 0) {
		MAP_ModCancel(m);
	}
	MAP_ModEnd(m);
	return (0);
}

static int
effect(void *p, MAP_Node *n)
{
	MAP_View *mv = TOOL(p)->mv;
	MAP *m = mv->map;
	MAP_Item *nref;
	RG_Transform *xf;
	int nmods = 0;

	MAP_ModNodeChg(m, mv->cx, mv->cy);

	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;
		
		nmods++;

		TAILQ_FOREACH(xf, &nref->transforms, transforms) {
			if (xf->type == RG_TRANSFORM_RGB_INVERT) {
				TAILQ_REMOVE(&nref->transforms, xf,
				    transforms);
				break;
			}
		}
		if (xf != NULL)
			continue;

		if ((xf = RG_TransformNew(RG_TRANSFORM_RGB_INVERT, 0, NULL))
		    == NULL) {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
			continue;
		}
		TAILQ_INSERT_TAIL(&nref->transforms, xf, transforms);
		break;
	}
	return (nmods);
}

static int
cursor(void *p, SDL_Rect *rd)
{
	Uint8 c[4] = { 255, 255, 255, 64 };

	agPrim.rect_blended(TOOL(p)->mv, rd->x, rd->y, rd->w, rd->h, 
	    c, AG_ALPHA_OVERLAY);
	return (1);
}

const MAP_ToolOps mapInvertOps = {
	"Invert", N_("Invert RGB values of tile"),
	INVERT_TOOL_ICON,
	sizeof(MAP_Tool),
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
