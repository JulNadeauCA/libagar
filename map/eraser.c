/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/checkbox.h>

#include <agar/map/map.h>

static int erase_all = 0;	/* XXX instance */
static int all_layers = 0;

static int
DeleteNoderefs(MAP_Tool *_Nonnull t, AG_KeySym key, int state,
    void *_Nullable arg)
{
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	Uint x, y;

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
Init(void *_Nonnull p)
{
	MAP_ToolBindKey(p, 0, AG_KEY_DELETE, DeleteNoderefs, NULL);
	MAP_ToolPushStatus(p,
	    _("Select a node element and use $(L) to delete."));
}

static void
EditPane(void *_Nonnull p, void *_Nonnull con)
{
	AG_CheckboxNewInt(con, 0, _("Erase all elements"), &erase_all);
	AG_CheckboxNewInt(con, 0, _("Apply to all layers"), &all_layers);
}

static int
MouseButtonDown(void *_Nonnull p, int x, int y, int btn)
{
	MAP_ModBegin(TOOL(p)->mv->map);
	return (0);
}

static int
MouseButtonUp(void *_Nonnull p, int x, int y, int btn)
{
	MAP_Tool *t = p;
	MAP_View *mv = t->mv;
	MAP *m = mv->map;

	if (m->nMods == 0) {
		MAP_ModCancel(m);
	}
	MAP_ModEnd(m);
	return (0);
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull n)
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
		Free(r);
		nmods++;

		if (!erase_all)
			break;
	}
	return (nmods);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
	AG_Color c;

	AG_ColorRGBA_8(&c, 255,0,0,64);
	AG_DrawRectBlended(TOOL(p)->mv, rd, &c,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	return (1);
}

const MAP_ToolOps mapEraserOps = {
	"Eraser", N_("Remove node elements."),
	&mapIconEraser,
	sizeof(MAP_Tool),
	0,
	1,
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
