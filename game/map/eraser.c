/*	$Csoft: eraser.c,v 1.8 2005/08/27 04:34:05 vedge Exp $	*/

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

#include <engine/widget/checkbox.h>
#include <engine/widget/primitive.h>

static int erase_all = 0;
static int all_layers = 0;

static int
delete_noderefs(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	AG_Mapview *mv = t->mv;
	AG_Map *m = mv->map;
	int x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			AG_Node *node = &m->map[y][x];
			AG_Nitem *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer != m->cur_layer ||
				    r->flags & AG_NITEM_NOSAVE) {
					continue;
				}
				if (r->flags & AG_NITEM_SELECTED)
					AG_NodeDelItem(m, node, r);
			}
		}
	}
	return (0);
}

static void
eraser_init(void *p)
{
	AG_MaptoolBindKey(p, 0, SDLK_DELETE, delete_noderefs, NULL);
	AG_MaptoolPushStatus(p,
	    _("Select a node element and use $(L) to delete."));
}

static void
eraser_pane(void *p, void *con)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNew(con, _("Erase all elements"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &erase_all);
	
	cb = AG_CheckboxNew(con, _("Apply to all layers"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &all_layers);
}

static int
mousebuttondown(void *p, int x, int y, int btn)
{
	AG_MapmodBegin(TOOL(p)->mv->map);
	return (0);
}

static int
mousebuttonup(void *p, int x, int y, int btn)
{
	AG_Maptool *t = p;
	AG_Mapview *mv = t->mv;
	AG_Map *m = mv->map;

	if (m->nmods == 0) {
		AG_MapmodCancel(m);
	}
	AG_MapmodEnd(m);
	return (0);
}

static int
eraser_effect(void *p, AG_Node *n)
{
	AG_Maptool *t = p;
	AG_Mapview *mv = t->mv;
	AG_Map *m = mv->map;
	AG_Nitem *r;
	int nmods = 0;

	AG_MapmodNodeChg(m, mv->cx, mv->cy);

	TAILQ_FOREACH(r, &n->nrefs, nrefs) {
		if (!all_layers &&
		    r->layer != m->cur_layer)
			continue;

		TAILQ_REMOVE(&n->nrefs, r, nrefs);
		AG_NitemDestroy(m, r);
		Free(r, M_MAP_NITEM);
		nmods++;

		if (!erase_all)
			break;
	}
	return (nmods);
}

static int
eraser_cursor(void *p, SDL_Rect *rd)
{
	Uint8 c[4] = { 255, 0, 0, 64 };

	agPrim.rect_blended(TOOL(p)->mv, rd->x, rd->y, rd->w, rd->h, 
	    c, AG_ALPHA_OVERLAY);
	return (1);
}

const AG_MaptoolOps agMapEraserOps = {
	"Eraser", N_("Remove node elements."),
	ERASER_TOOL_ICON,
	sizeof(AG_Maptool),
	0,
	eraser_init,
	NULL,			/* destroy */
	eraser_pane,
	NULL,			/* edit */
	eraser_cursor,
	eraser_effect,

	NULL,			/* mousemotion */
	mousebuttondown,
	mousebuttonup,
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
