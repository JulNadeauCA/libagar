/*
 * Copyright (c) 2003-2021 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/text.h>

#include <agar/map/map.h>
#include <agar/map/nodesel.h>

/* Begin a rectangular selection of nodes. */
void
MAP_NodeselBegin(MAP_View *mv)
{
	mv->esel.set = 0;
	mv->msel.set = 1;
	mv->msel.x = mv->cx;
	mv->msel.y = mv->cy;
	mv->msel.xOffs = 1;
	mv->msel.yOffs = 1;
}

/* Apply the temporary rectangular selection. */
void
MAP_NodeselEnd(MAP_View *mv)
{
	int excess;

	mv->esel.x = mv->msel.x;
	mv->esel.y = mv->msel.y;
	mv->esel.w = mv->msel.xOffs;
	mv->esel.h = mv->msel.yOffs;

	if (mv->msel.xOffs < 0) {
		mv->esel.x += mv->msel.xOffs;
		mv->esel.w = -mv->msel.xOffs;
	}
	if (mv->msel.yOffs < 0) {
		mv->esel.y += mv->msel.yOffs;
		mv->esel.h = -mv->msel.yOffs;
	}

	if ((excess = (mv->esel.x + mv->esel.w) - mv->map->w) > 0) {
		if (excess < mv->esel.w)
			mv->esel.w -= excess;
	}
	if ((excess = (mv->esel.y + mv->esel.h) - mv->map->h) > 0) {
		if (excess < mv->esel.h)
			mv->esel.h -= excess;
	}

	if (mv->esel.x < 0) {
		mv->esel.w += mv->esel.x;
		mv->esel.x = 0;
	}
	if (mv->esel.y < 0) {
		mv->esel.h += mv->esel.y;
		mv->esel.y = 0;
	}

	mv->esel.set = 1;

	MAP_ViewStatus(mv, _("Selected area %d,%d (%dx%d)"),
	    mv->esel.x, mv->esel.y, mv->esel.w, mv->esel.h);
}

/* Begin displacement of the node selection. */
void
MAP_NodeselBeginMove(MAP_View *mv)
{
	MAP *mapSrc = mv->map;
	MAP *mapTmp = &mv->esel.map;
	int sx,sy, x,y;

	AG_ObjectInit(mapTmp, &mapClass);
	OBJECT(mapTmp)->flags |= AG_OBJECT_STATIC;

	if (MAP_AllocNodes(mapTmp, mv->esel.w, mv->esel.h) == -1) {
		goto fail;
	}
	if (MAP_PushLayer(mapSrc, _("(Floating selection)")) == -1) {
		goto fail;
	}
	MAP_ModBegin(mapSrc);

	for (y = 0, sy = mv->esel.y;
	     y < mv->esel.h;
	     y++, sy++) {
		for (x = 0, sx = mv->esel.x;
		     x < mv->esel.w;
		     x++, sx++) {
			MAP_Node *nSrc = &mapSrc->map[sy][sx];
			MAP_Node *nTmp = &mapTmp->map[y][x];

			MAP_ModNodeChg(mapSrc, sx, sy);

			MAP_NodeCopy(mapSrc, nSrc, mapSrc->layerCur,
			             mapTmp, nTmp, 0);

			MAP_NodeSwapLayers(mapSrc, nSrc, mapSrc->layerCur,
			                   mapSrc->nLayers - 1);
		}
	}
	
	mv->esel.moving = 1;
	return;
fail:
	AG_ObjectDestroy(mapTmp);
}

void
MAP_NodeselUpdateMove(MAP_View *mv, int xRel, int yRel)
{
	MAP *mapDst = mv->map;
	MAP *mapTmp = &mv->esel.map;
	int x,y, dx,dy;

	if (mv->esel.x+xRel < 0 || mv->esel.x+mv->esel.w+xRel > (int)mapDst->w)
		xRel = 0;
	if (mv->esel.y+yRel < 0 || mv->esel.y+mv->esel.h+yRel > (int)mapDst->h)
		yRel = 0;
	
	for (y = 0, dy = mv->esel.y;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x;
		     x < mv->esel.w;
		     x++)
			MAP_NodeRemoveAll(mapDst, &mapDst->map[dy][dx],
			                  mapDst->nLayers-1);
	}

	for (y = 0, dy = mv->esel.y+yRel;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x+xRel;
		     x < mv->esel.w;
		     x++, dx++) {
			MAP_Node *nTmp = &mapTmp->map[y][x];
			MAP_Node *nDst = &mapDst->map[dy][dx];
	
			MAP_ModNodeChg(mapDst, dx, dy);

			MAP_NodeCopy(mapTmp, nTmp, 0, mapDst, nDst,
			             mapDst->nLayers - 1);
		}
	}
	
	mv->esel.x += xRel;
	mv->esel.y += yRel;
}

void
MAP_NodeselEndMove(MAP_View *mv)
{
	MAP *mapDst = mv->map;
	MAP *mapTmp = &mv->esel.map;
	int dx,dy, x,y;

	for (y = 0, dy = mv->esel.y;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x;
		     x < mv->esel.w;
		     x++, dx++) {
			MAP_Node *node = &mapDst->map[dy][dx];
			MAP_Item *mi;

			TAILQ_FOREACH(mi, &node->items, items)
				if (mi->layer == mapDst->nLayers-1)
					mi->layer = mapDst->layerCur;
		}
	}

	MAP_PopLayer(mapDst);
	MAP_ModEnd(mapDst);
	
	AG_ObjectReset(mapTmp);
	AG_ObjectDestroy(mapTmp);
	mv->esel.moving = 0;
}

/* Copy the selection to the copy buffer. */
int
MAP_NodeselCopy(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = tool->mv;
	MAP *mapBuf = &mapEditor.copybuf;
	MAP *mapDst = mv->map;
	int sx,sy, dx,dy;

	if (!mv->esel.set) {
		AG_TextMsg(AG_MSG_ERROR, _("There is no selection to copy."));
		return (0);
	}
	if (mapBuf->map != NULL) {
		MAP_FreeNodes(mapBuf);
	}
	if (MAP_AllocNodes(mapBuf, mv->esel.w, mv->esel.h) == -1) {
		AG_TextMsgFromError();
		return (0);
	}

	for (sy = mv->esel.y, dy = 0;
	     sy < mv->esel.y + mv->esel.h;
	     sy++, dy++) {
		for (sx = mv->esel.x, dx = 0;
		     sx < mv->esel.x + mv->esel.w;
		     sx++, dx++)
			MAP_NodeCopy(mapDst, &mapDst->map[sy][sx], mapDst->layerCur,
			             mapBuf, &mapBuf->map[dy][dx], 0);
	}
	return (1);
}

int
MAP_NodeselPaste(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = tool->mv;
	MAP *mapBuf = &mapEditor.copybuf;
	MAP *mapDst = mv->map;
	int sx, sy, dx, dy;
	
	if (mapBuf->map == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("The copy buffer is empty!"));
		return (0);
	}

	if (mv->esel.set) {
		dx = mv->esel.x;
		dy = mv->esel.y;
	} else {
		if (mv->cx != -1 && mv->cy != -1) {
			dx = mv->cx;
			dy = mv->cy;
		} else {
			dx = 0;
			dy = 0;
		}
	}

	Debug(mapDst, "Pasting [%dx%d] map at [%d,%d]\n", mapBuf->w,
	    mapBuf->h, dx, dy);

	for (sy = 0, dy = mv->esel.y;
	     sy < (int)mapBuf->h && dy < (int)mapDst->h;
	     sy++, dy++) {
		for (sx = 0, dx = mv->esel.x;
		     sx < (int)mapBuf->w && dx < (int)mapDst->w;
		     sx++, dx++)
			MAP_NodeCopy(mapBuf, &mapBuf->map[sy][sx], 0,
			             mapDst, &mapDst->map[dy][dx],
			             mapDst->layerCur);
	}
	return (1);
}

int
MAP_NodeselKill(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = tool->mv;
	MAP *map = mv->map;
	int x, y;

	if (!mv->esel.set)
		return (0);
	
	Debug(map, "Deleting region [%d,%d]+[%d,%d]\n", mv->esel.x, mv->esel.y,
	    mv->esel.w, mv->esel.h);

	for (y = mv->esel.y; y < mv->esel.y + mv->esel.h; y++) {
		for (x = mv->esel.x; x < mv->esel.x + mv->esel.w; x++)
			MAP_NodeRemoveAll(map, &map->map[y][x], map->layerCur);
	}
	return (1);
}

int
MAP_NodeselCut(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = tool->mv;

	if (!mv->esel.set) {
		AG_TextMsg(AG_MSG_ERROR, _("There is no selection to cut."));
		return (0);
	}
	MAP_NodeselCopy(tool, 0, 1, NULL);
	MAP_NodeselKill(tool, 0, 1, NULL);
	return (1);
}

static void
Init(void *_Nonnull tool)
{
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_C,      MAP_NodeselCopy,  NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_V,      MAP_NodeselPaste, NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_X,      MAP_NodeselCut,   NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_K,      MAP_NodeselKill,  NULL);
	MAP_ToolBindKey(tool, 0,              AG_KEY_DELETE, MAP_NodeselKill,  NULL);
	
	MAP_ToolPushStatus(tool,
	    _("Select a rectangle of nodes with $(L). Drag to displace node "
	       "elements."));
}

const MAP_ToolOps mapNodeselOps = {
	"Nodesel", N_("Select node(s)"),
	&mapIconSelectNode,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
