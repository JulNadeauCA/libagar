/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <gui/checkbox.h>
#include <gui/primitive.h>

#include "map.h"
#include "mapedit.h"
#include "icons.h"

static int erase_all = 0;
static int all_layers = 0;

static int
delete_noderefs(MAP_Tool *t, SDLKey key, int state, void *arg)
{
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	int x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer != m->cur_layer ||
				    r->flags & MAP_ITEM_NOSAVE) {
					continue;
				}
				if (r->flags & MAP_ITEM_SELECTED)
					MAP_NodeDelItem(m, node, r);
			}
		}
	}
	return (0);
}

static void
Init(void *p)
{
	MAP_ToolBindKey(p, 0, SDLK_DELETE, delete_noderefs, NULL);
	MAP_ToolPushStatus(p,
	    _("Select a node element and use $(L) to delete."));
}

static void
EditPane(void *p, void *con)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNew(con, 0, _("Erase all elements"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &erase_all);
	
	cb = AG_CheckboxNew(con, 0, _("Apply to all layers"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &all_layers);
}

static int
MouseButtonDown(void *p, int x, int y, int btn)
{
	MAP_ModBegin(TOOL(p)->mv->map);
	return (0);
}

static int
MouseButtonUp(void *p, int x, int y, int btn)
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
Effect(void *p, MAP_Node *n)
{
	MAP_Tool *t = p;
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	MAP_Item *r;
	int nmods = 0;

	MAP_ModNodeChg(m, mv->cx, mv->cy);

	TAILQ_FOREACH(r, &n->nrefs, nrefs) {
		if (!all_layers &&
		    r->layer != m->cur_layer)
			continue;

		TAILQ_REMOVE(&n->nrefs, r, nrefs);
		MAP_ItemDestroy(m, r);
		Free(r, M_MAP);
		nmods++;

		if (!erase_all)
			break;
	}
	return (nmods);
}

static int
Cursor(void *p, SDL_Rect *rd)
{
	Uint8 c[4] = { 255, 0, 0, 64 };

	agPrim.rect_blended(TOOL(p)->mv, rd->x, rd->y, rd->w, rd->h, 
	    c, AG_ALPHA_OVERLAY);
	return (1);
}

const MAP_ToolOps mapEraserOps = {
	"Eraser", N_("Remove node elements."),
	&mapIconEraser,
	sizeof(MAP_Tool),
	0,
	Init,
	NULL,			/* destroy */
	EditPane,
	NULL,			/* edit */
	Cursor,
	Effect,
	NULL,			/* mousemotion */
	MouseButtonDown,
	MouseButtonUp,
	NULL,			/* keydown */
	NULL			/* keyup */
};
