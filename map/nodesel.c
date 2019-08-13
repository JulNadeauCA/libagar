/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
	mv->msel.xoffs = 1;
	mv->msel.yoffs = 1;
}

/* Apply the temporary rectangular selection. */
void
MAP_NodeselEnd(MAP_View *mv)
{
	int excess;

	mv->esel.x = mv->msel.x;
	mv->esel.y = mv->msel.y;
	mv->esel.w = mv->msel.xoffs;
	mv->esel.h = mv->msel.yoffs;

	if (mv->msel.xoffs < 0) {
		mv->esel.x += mv->msel.xoffs;
		mv->esel.w = -mv->msel.xoffs;
	}
	if (mv->msel.yoffs < 0) {
		mv->esel.y += mv->msel.yoffs;
		mv->esel.h = -mv->msel.yoffs;
	}

	if ((excess = (mv->esel.x + mv->esel.w) - mv->map->mapw) > 0) {
		if (excess < mv->esel.w)
			mv->esel.w -= excess;
	}
	if ((excess = (mv->esel.y + mv->esel.h) - mv->map->maph) > 0) {
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
	MAP *mSrc = mv->map;
	MAP *mTmp = &mv->esel.map;
	int sx, sy, x, y;

	AG_ObjectInit(mTmp, &mapClass);
	OBJECT(mTmp)->flags |= AG_OBJECT_STATIC;

	if (MAP_AllocNodes(mTmp, mv->esel.w, mv->esel.h) == -1) {
		goto fail;
	}
	if (MAP_PushLayer(mSrc, _("(Floating selection)")) == -1) {
		goto fail;
	}
	MAP_ModBegin(mSrc);

	for (y = 0, sy = mv->esel.y;
	     y < mv->esel.h;
	     y++, sy++) {
		for (x = 0, sx = mv->esel.x;
		     x < mv->esel.w;
		     x++, sx++) {
			MAP_Node *nSrc = &mSrc->map[sy][sx];
			MAP_Node *nTmp = &mTmp->map[y][x];

			MAP_ModNodeChg(mSrc, sx, sy);

			MAP_NodeCopy(mSrc, nSrc, mSrc->cur_layer, mTmp, nTmp,
			    0);
			MAP_NodeSwapLayers(mSrc, nSrc, mSrc->cur_layer,
			    mSrc->nLayers-1);
		}
	}
	
	mv->esel.moving = 1;
	return;
fail:
	AG_ObjectDestroy(mTmp);
}

void
MAP_NodeselUpdateMove(MAP_View *mv, int xRel, int yRel)
{
	MAP *mDst = mv->map;
	MAP *mTmp = &mv->esel.map;
	int x, y, dx, dy;

	if (mv->esel.x+xRel < 0 || mv->esel.x+mv->esel.w+xRel > (int)mDst->mapw)
		xRel = 0;
	if (mv->esel.y+yRel < 0 || mv->esel.y+mv->esel.h+yRel > (int)mDst->maph)
		yRel = 0;
	
	for (y = 0, dy = mv->esel.y;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x;
		     x < mv->esel.w;
		     x++) {
			MAP_NodeRemoveAll(mDst, &mDst->map[dy][dx],
			    mDst->nLayers-1);
		}
	}

	for (y = 0, dy = mv->esel.y+yRel;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x+xRel;
		     x < mv->esel.w;
		     x++, dx++) {
			MAP_Node *nTmp = &mTmp->map[y][x];
			MAP_Node *nDst = &mDst->map[dy][dx];
	
			MAP_ModNodeChg(mDst, dx, dy);
			MAP_NodeCopy(mTmp, nTmp, 0, mDst, nDst,
			    mDst->nLayers-1);
		}
	}
	
	mv->esel.x += xRel;
	mv->esel.y += yRel;
}

void
MAP_NodeselEndMove(MAP_View *mv)
{
	MAP *mDst = mv->map;
	MAP *mTmp = &mv->esel.map;
	int dx, dy, x, y;

	for (y = 0, dy = mv->esel.y;
	     y < mv->esel.h;
	     y++, dy++) {
		for (x = 0, dx = mv->esel.x;
		     x < mv->esel.w;
		     x++, dx++) {
			MAP_Node *node = &mDst->map[dy][dx];
			MAP_Item *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer == mDst->nLayers-1)
					nref->layer = mDst->cur_layer;
			}
		}
	}

	MAP_PopLayer(mDst);
	MAP_ModEnd(mDst);
	
	AG_ObjectReset(mTmp);
	AG_ObjectDestroy(mTmp);
	mv->esel.moving = 0;
}

/* Copy the selection to the copy buffer. */
int
MAP_NodeselCopy(MAP_Tool *t, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = t->mv;
	MAP *copybuf = &mapEditor.copybuf;
	MAP *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		AG_TextMsg(AG_MSG_ERROR, _("There is no selection to copy."));
		return (0);
	}
	if (copybuf->map != NULL) {
		MAP_FreeNodes(copybuf);
	}
	if (MAP_AllocNodes(copybuf, mv->esel.w, mv->esel.h) == -1) {
		AG_TextMsgFromError();
		return (0);
	}

	for (sy = mv->esel.y, dy = 0;
	     sy < mv->esel.y + mv->esel.h;
	     sy++, dy++) {
		for (sx = mv->esel.x, dx = 0;
		     sx < mv->esel.x + mv->esel.w;
		     sx++, dx++) {
			MAP_NodeCopy(m, &m->map[sy][sx], m->cur_layer, copybuf,
			    &copybuf->map[dy][dx], 0);
		}
	}
	return (1);
}

int
MAP_NodeselPaste(MAP_Tool *t, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = t->mv;
	MAP *copybuf = &mapEditor.copybuf;
	MAP *m = mv->map;
	int sx, sy, dx, dy;
	
	if (copybuf->map == NULL) {
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

	Debug(m, "Pasting [%dx%d] map at [%d,%d]\n", copybuf->mapw,
	    copybuf->maph, dx, dy);

	for (sy = 0, dy = mv->esel.y;
	     sy < (int)copybuf->maph && dy < (int)m->maph;
	     sy++, dy++) {
		for (sx = 0, dx = mv->esel.x;
		     sx < (int)copybuf->mapw && dx < (int)m->mapw;
		     sx++, dx++) {
			MAP_NodeCopy(copybuf, &copybuf->map[sy][sx], 0,
			    m, &m->map[dy][dx], m->cur_layer);
		}
	}
	return (1);
}

int
MAP_NodeselKill(MAP_Tool *t, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	int x, y;

	if (!mv->esel.set)
		return (0);
	
	Debug(m, "Deleting region [%d,%d]+[%d,%d]\n", mv->esel.x, mv->esel.y,
	    mv->esel.w, mv->esel.h);

	for (y = mv->esel.y; y < mv->esel.y + mv->esel.h; y++) {
		for (x = mv->esel.x; x < mv->esel.x + mv->esel.w; x++) {
			MAP_NodeRemoveAll(m, &m->map[y][x], m->cur_layer);
		}
	}
	return (1);
}

int
MAP_NodeselCut(MAP_Tool *t, AG_KeySym key, int state, void *arg)
{
	MAP_View *mv = t->mv;

	if (!mv->esel.set) {
		AG_TextMsg(AG_MSG_ERROR, _("There is no selection to cut."));
		return (0);
	}
	MAP_NodeselCopy(t, 0, 1, NULL);
	MAP_NodeselKill(t, 0, 1, NULL);
	return (1);
}

static void
Init(void *_Nonnull p)
{
	MAP_ToolBindKey(p, AG_KEYMOD_CTRL, AG_KEY_C, MAP_NodeselCopy, NULL);
	MAP_ToolBindKey(p, AG_KEYMOD_CTRL, AG_KEY_V, MAP_NodeselPaste, NULL);
	MAP_ToolBindKey(p, AG_KEYMOD_CTRL, AG_KEY_X, MAP_NodeselCut, NULL);
	MAP_ToolBindKey(p, AG_KEYMOD_CTRL, AG_KEY_K, MAP_NodeselKill, NULL);
	MAP_ToolBindKey(p, 0, AG_KEY_DELETE, MAP_NodeselKill, NULL);
	
	MAP_ToolPushStatus(p,
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
