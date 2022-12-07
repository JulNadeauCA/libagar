/*
 * Copyright (c) 2003-2022 Julien Nadeau Carriere <vedge@csoft.net>
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

static int mapFlipSelection = 0;	/* XXX instance */

static void
Init(void *_Nonnull obj)
{
	MAP_ToolPushStatus(obj,
	    _("Select an element and Left Click to Mirror or Right Click to Flip."));
}

static void
EditPane(void *_Nonnull obj, void *_Nonnull box)
{
	AG_CheckboxNewInt(box, 0, _("Flip entire selection"), &mapFlipSelection);
}

static void
ToggleXform(MAP_Item *_Nonnull mi, int type)
{
	RG_Transform *xf;

	TAILQ_FOREACH(xf, &mi->transforms, transforms) {
		if (xf->type == type) {
			TAILQ_REMOVE(&mi->transforms, xf, transforms);
			return;
		}
	}
	if ((xf = RG_TransformNew(type, 0, NULL)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	TAILQ_INSERT_TAIL(&mi->transforms, xf, transforms);
}

static int
MouseButtonDown(void *_Nonnull obj, int xMap, int yMap, int b)
{
	MAP_Tool *tool = obj;
	MAP_View *mv = tool->mv;
	MAP *map = mv->map;
	int xSel = mv->mx + mv->mouse.x;
	int ySel = mv->my + mv->mouse.y;
	int w=1, h=1, x,y;

	if (!mapFlipSelection ||
	    MAP_ViewGetSelection(mv, &xSel, &ySel, &w, &h) == -1) {
		if (xSel < 0 || xSel >= (int)map->w ||
		    ySel < 0 || ySel >= (int)map->h)
			return (0);
	}

	MAP_BeginRevision(map);

	for (y = ySel; y < ySel+h; y++) {
		for (x = xSel ; x < xSel+w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi;

			MAP_NodeRevision(map, x,y, map->undo, map->nUndo);
			
			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer != map->layerCur)
					continue;

				if (b == AG_MOUSE_LEFT) {
					ToggleXform(mi, RG_TRANSFORM_MIRROR);
				} else if (b == AG_MOUSE_RIGHT) {
					ToggleXform(mi, RG_TRANSFORM_FLIP);
				}
			}
		}
	}

	MAP_CommitRevision(map);
	return (1);
}

static int
Cursor(void *_Nonnull obj, AG_Rect *_Nonnull rd)
{
	AG_Color c;

	AG_ColorRGBA_8(&c, 255,255,0,64);
	AG_DrawRectBlended(TOOL(obj)->mv, rd, &c, AG_ALPHA_SRC,
	                                          AG_ALPHA_ONE_MINUS_SRC);
	return (1);
}

const MAP_ToolOps mapFlipOps = {
	"Flip", N_("Flip/mirror node element"),
	&mapIconFlip,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	EditPane,
	NULL,			/* edit */
	Cursor,
	NULL,			/* effect */
	NULL,			/* mousemotion */
	MouseButtonDown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
