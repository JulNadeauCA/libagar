/*	$Csoft: flip.c,v 1.10 2005/08/27 04:34:05 vedge Exp $	*/

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

#include <engine/widget/checkbox.h>
#include <engine/widget/primitive.h>

#include "map.h"
#include "mapedit.h"

static int multi_ind = 0;

static void
flip_init(void *p)
{
	AG_MaptoolPushStatus(p,
	    _("Select element and use $(L)=Mirror, $(R)=Flip"));
}

static void
flip_pane(void *p, void *con)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNew(con, _("Flip entire selection"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &multi_ind);
}

static void
toggle_transform(AG_Nitem *nref, int type)
{
	AG_Transform *trans;

	TAILQ_FOREACH(trans, &nref->transforms, transforms) {
		if (trans->type == type) {
			TAILQ_REMOVE(&nref->transforms, trans, transforms);
			return;
		}
	}
	if ((trans = AG_TransformNew(type, 0, NULL)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		return;
	}
	TAILQ_INSERT_TAIL(&nref->transforms, trans, transforms);
}

static int
flip_mousebuttondown(void *p, int xmap, int ymap, int b)
{
	AG_Maptool *t = p;
	AG_Mapview *mv = t->mv;
	AG_Map *m = mv->map;
	int selx = mv->mx + mv->mouse.x;
	int sely = mv->my + mv->mouse.y;
	int w = 1;
	int h = 1;
	int x, y;

	if (!multi_ind ||
	    AG_MapviewGetSelection(mv, &selx, &sely, &w, &h) == -1) {
		if (selx < 0 || selx >= m->mapw ||
		    sely < 0 || sely >= m->maph)
			return (0);
	}

	AG_MapmodBegin(m);

	for (y = sely; y < sely+h; y++) {
		for (x = selx; x < selx+w; x++) {
			AG_Node *node = &m->map[y][x];
			AG_Nitem *nref;

			AG_MapmodNodeChg(m, x, y);
			
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != m->cur_layer)
					continue;

				if (b == SDL_BUTTON_LEFT) {
					toggle_transform(nref,
					    AG_TRANSFORM_MIRROR);
				} else if (b == SDL_BUTTON_RIGHT) {
					toggle_transform(nref,
					    AG_TRANSFORM_FLIP);
				}
			}
		}
	}

	AG_MapmodEnd(m);
	return (1);
}

static int
flip_cursor(void *p, SDL_Rect *rd)
{
	Uint8 c[4] = { 255, 255, 0, 64 };

	agPrim.rect_blended(TOOL(p)->mv, rd->x, rd->y, rd->w, rd->h, 
	    c, AG_ALPHA_OVERLAY);
	return (1);
}

const AG_MaptoolOps agMapFlipOps = {
	"Flip", N_("Flip/mirror node element"),
	FLIP_TOOL_ICON,
	sizeof(AG_Maptool),
	0,
	flip_init,
	NULL,			/* destroy */
	flip_pane,
	NULL,			/* edit */
	flip_cursor,
	NULL,			/* effect */

	NULL,			/* mousemotion */
	flip_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
