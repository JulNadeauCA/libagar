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

/*
 * Select tool. Allows the user to select nodes, items and objects.
 * Implements Move, Copy/Paste and Clear.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

#include <agar/map/map.h>
#include <agar/map/nodesel.h>

/* Define a selection of nodes. */
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

	MAP_ViewStatus(mv, _("Selected (%dx%d) nodes."),
	    mv->esel.w, mv->esel.h);
}

/* Begin displacement of the node selection. */
void
MAP_NodeselMoveBegin(MAP_View *mv)
{
	MAP *map = mv->map;
	MAP *mapTmp = &mv->esel.map;
	int x,y, xTmp,yTmp;
	Uint layerCur;
	const int xSel = mv->esel.x;
	const int ySel = mv->esel.y;
	const int wSel = mv->esel.w;
	const int hSel = mv->esel.h;

	AG_ObjectInitStatic(mapTmp, &mapClass);

	if (MAP_AllocNodes(mapTmp, wSel,hSel) == -1) {
		AG_ObjectDestroy(mapTmp);
		return;
	}

	AG_ObjectLock(map);

	layerCur = map->layerCur;
	
	MAP_BeginRevision(map);

	for (yTmp = 0, y = ySel;
	     yTmp < hSel;
	     yTmp++, y++) {
		for (xTmp = 0, x = xSel;
		     xTmp < wSel;
		     xTmp++, x++) {
			MAP_Node *nodeSrc = &map->map[y][x];

			MAP_NodeRevision(map, x,y, map->undo, map->nUndo);
			MAP_NodeCopy(mapTmp, &mapTmp->map[yTmp][xTmp], layerCur,
			             nodeSrc, layerCur);
			MAP_NodeClear(map, nodeSrc, layerCur);
		}
	}
	
	mv->esel.flags |= MAP_VIEW_SELECTION_MOVING;
	mv->esel.xOrig = mv->esel.x;
	mv->esel.yOrig = mv->esel.y;

	AG_ObjectUnlock(map);
}

void
MAP_NodeselMoveUpdate(MAP_View *mv, int xRel, int yRel)
{
	mv->esel.x += xRel;
	mv->esel.y += yRel;

	if (mv->esel.x + mv->esel.w >= mv->map->w)
		mv->esel.x = mv->map->w - mv->esel.w;
	if (mv->esel.x < 0)
		mv->esel.x = 0;

	if (mv->esel.y + mv->esel.h >= mv->map->h)
		mv->esel.y = mv->map->h - mv->esel.h;
	if (mv->esel.y < 0)
		mv->esel.y = 0;

	MAP_ViewStatus(mv, _("Move (%dx%d) nodes to ["
	                     AGSI_BOLD "%d,%d" AGSI_RST "]."),
	    mv->esel.w, mv->esel.h,
	    mv->esel.x, mv->esel.y);
}

void
MAP_NodeselMoveEnd(MAP_View *mv)
{
	MAP *map = mv->map;
	MAP *mapTmp = &mv->esel.map;
	int x,y, xDst,yDst;
	const Uint layerCur = map->layerCur;
	const int hSel = mv->esel.h;
	const int wSel = mv->esel.w;
	const int xSel = mv->esel.x;
	const int ySel = mv->esel.y;

	for (y=0, yDst=ySel;
	     y < hSel;
	     y++, yDst++) {
		for (x=0, xDst=xSel;
		     x < wSel;
		     x++, xDst++) {
			MAP_Node *nodeDst = &map->map[yDst][xDst];

			MAP_NodeRevision(map, xDst,yDst, map->undo, map->nUndo);
			MAP_NodeClear(map, nodeDst, layerCur);
			MAP_NodeCopy(map, nodeDst, layerCur,
			    &mapTmp->map[y][x], layerCur);
		}
	}

	MAP_CommitRevision(map);

	AG_ObjectDestroy(mapTmp);

	mv->esel.flags &= ~(MAP_VIEW_SELECTION_MOVING);
	mv->esel.flags |= MAP_VIEW_SELECTION_MOVED;

	MAP_ViewStatus(mv, _("Moved (%dx%d) nodes to ["
	                     AGSI_BOLD "%d,%d" AGSI_RST "]."),
	    wSel,hSel, xSel,ySel);
}

/* Copy the selection to the map clipboard. */
int
MAP_NodeselCopy(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_NodeselTool *selTool = (MAP_NodeselTool *)tool;
	MAP_View *mv = tool->mv;
	MAP *map = mv->map, *mapCopy = &selTool->mapCopy;
	int xSrc,ySrc, x,y;
	const int xSel = mv->esel.x;
	const int ySel = mv->esel.y;
	const int wSel = mv->esel.w;
	const int hSel = mv->esel.h;

	if (!mv->esel.set) {
		AG_TextMsg(AG_MSG_ERROR, _("There is no selection to copy."));
		return (0);
	}
	if (mapCopy->map != NULL) {
		MAP_FreeNodes(mapCopy);
	}
	if (MAP_AllocNodes(mapCopy, wSel,hSel) == -1) {
		AG_TextMsgFromError();
		return (0);
	}

	for (ySrc=ySel, y=0;
	     ySrc < ySel+hSel;
	     ySrc++, y++) {
		for (xSrc=xSel, x=0;
		     xSrc < xSel+wSel;
		     xSrc++, x++)
			MAP_NodeCopy(map, &map->map[ySrc][xSrc], map->layerCur,
			    &mapCopy->map[y][x], 0);
	}

	MAP_ViewStatus(mv, _("Copied (%dx%d) nodes to clipboard."), wSel,hSel);
	return (1);
}

int
MAP_NodeselPaste(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_NodeselTool *selTool = (MAP_NodeselTool *)tool;
	MAP_View *mv = tool->mv;
	MAP *map = mv->map, *mapCopy = &selTool->mapCopy;
	const int wCopy = (int)mapCopy->w;
	const int hCopy = (int)mapCopy->h;
	const int wDst = (int)map->w;
	const int hDst = (int)map->h;
	const int xSel = mv->esel.x;
	const int ySel = mv->esel.y;
	int xSrc,ySrc, x,y;
	
	if (mapCopy->map == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("The copy buffer is empty!"));
		return (0);
	}

	if (mv->esel.set) {
		x = xSel;
		y = ySel;
	} else {
		if (mv->cx != -1 && mv->cy != -1) {
			x = mv->cx;
			y = mv->cy;
		} else {
			x = 0;
			y = 0;
		}
	}

	Debug(map, "Pasting [%dx%d] map at [%d,%d]\n", wCopy,hCopy, x,y);

	MAP_BeginRevision(map);

	for (ySrc=0, y=ySel;
	     ySrc < hCopy && y < hDst;
	     ySrc++, y++) {
		for (xSrc=0, x=xSel;
		     xSrc < wCopy && x < wDst;
		     xSrc++, x++) {
			MAP_NodeRevision(map, x,y, map->undo, map->nUndo);
			MAP_NodeCopy(map, &map->map[y][x], map->layerCur,
			    &mapCopy->map[ySrc][xSrc], 0);
		}
	}

	MAP_CommitRevision(map);

	MAP_ViewStatus(mv, _("Pasted (%dx%d) nodes from clipboard."), wCopy,hCopy);
	return (1);
}

int
MAP_NodeselClear(MAP_Tool *tool, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = tool->mv;
	MAP *map = mv->map;
	const int xSel = mv->esel.x;
	const int ySel = mv->esel.y;
	const int wSel = mv->esel.w;
	const int hSel = mv->esel.h;
	int x, y;

	if (!mv->esel.set)
		return (0);
	
	MAP_BeginRevision(map);

	for (y = ySel; y < ySel + hSel; y++) {
		for (x = xSel; x < xSel + wSel; x++) {
			MAP_NodeRevision(map, x,y, map->undo, map->nUndo);
			MAP_NodeClear(map, &map->map[y][x], map->layerCur);
		}
	}
	MAP_ViewStatus(mv, _("Cleared (%dx%d) nodes at "
	                     "[" AGSI_BOLD "%d,%d" AGSI_RST "]."),
	    wSel,hSel, xSel,ySel);

	MAP_CommitRevision(map);
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
	MAP_NodeselClear(tool, 0, 1, NULL);	/* Will create undo level */

	MAP_ViewStatus(mv, _("Cut (%dx%d) nodes at "
	                     "[" AGSI_BOLD "%d,%d" AGSI_RST "]."),
	    mv->esel.w, mv->esel.h, mv->esel.x, mv->esel.y);
	return (1);
}

static void
Init(void *_Nonnull tool)
{
	MAP_NodeselTool *selTool = (MAP_NodeselTool *)tool;

	/* Initialize the copy buffer (clipboard). */
	AG_ObjectInitStatic(&selTool->mapCopy, &mapClass);

	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_C,      MAP_NodeselCopy,  NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_V,      MAP_NodeselPaste, NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_X,      MAP_NodeselCut,   NULL);
	MAP_ToolBindKey(tool, AG_KEYMOD_CTRL, AG_KEY_K,      MAP_NodeselClear, NULL);
	MAP_ToolBindKey(tool, 0,              AG_KEY_DELETE, MAP_NodeselClear, NULL);
	
	MAP_ToolPushStatus(tool,
	    _("Select a rectangle of nodes with Left-Click and drag elements "
	      "to move them."));
}

static void
Destroy(void *_Nonnull tool)
{
	MAP_NodeselTool *selTool = (MAP_NodeselTool *)tool;

	AG_ObjectDestroy(&selTool->mapCopy);
}

const MAP_ToolOps mapNodeselOps = {
	"Nodesel", N_("Select node(s)"),
	&mapIconSelectNode,
	sizeof(MAP_NodeselTool),
	0,
	1,
	Init,
	Destroy,
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
