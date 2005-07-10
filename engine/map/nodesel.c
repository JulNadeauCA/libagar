/*	$Csoft: select.c,v 1.6 2005/06/21 08:09:07 vedge Exp $	*/

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

#include "map.h"
#include "mapedit.h"
#include "nodesel.h"

/* Begin a rectangular selection of nodes. */
void
nodesel_begin(struct mapview *mv)
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
nodesel_end(struct mapview *mv)
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

	mapview_status(mv, _("Selected area %d,%d (%dx%d)"),
	    mv->esel.x, mv->esel.y, mv->esel.w, mv->esel.h);
}

/* Begin displacement of the node selection. */
void
nodesel_begin_move(struct mapview *mv)
{
	struct map *mSrc = mv->map;
	struct map *mTmp = &mv->esel.map;
	int x, y;

	map_init(mTmp, "");

	if (map_alloc_nodes(mTmp, mv->esel.w, mv->esel.h) == -1)
		goto fail;

	if (map_push_layer(mSrc, _("(Floating selection)")) == -1)
		goto fail;

	for (y = 0; y < mv->esel.h; y++) {
		for (x = 0; x < mv->esel.w; x++) {
			struct node *nSrc = &mSrc->map[mv->esel.y+y]
			                              [mv->esel.x+x];
			struct node *nTmp = &mTmp->map[y][x];

			node_copy(mSrc, nSrc, mSrc->cur_layer, mTmp, nTmp, 0);
			node_swap_layers(mSrc, nSrc, mSrc->cur_layer,
			    mSrc->nlayers-1);
		}
	}
	
	mv->esel.moving = 1;
	return;
fail:
	map_destroy(mTmp);
}

void
nodesel_update_move(struct mapview *mv, int xRel, int yRel)
{
	struct map *mDst = mv->map;
	struct map *mTmp = &mv->esel.map;
	int x, y;

	if (mv->esel.x+xRel < 0 || mv->esel.x+mv->esel.w+xRel > mDst->mapw)
		xRel = 0;
	if (mv->esel.y+yRel < 0 || mv->esel.y+mv->esel.h+yRel > mDst->maph)
		yRel = 0;
	
	for (y = 0; y < mv->esel.h; y++) {
		for (x = 0; x < mv->esel.w; x++) {
			node_clear(mDst,
			    &mDst->map[mv->esel.y+y][mv->esel.x+x],
			    mDst->nlayers-1);
		}
	}

	for (y = 0; y < mv->esel.h; y++) {
		for (x = 0; x < mv->esel.w; x++) {
			struct node *nTmp = &mTmp->map[y][x];
			struct node *nDst = &mDst->map[mv->esel.y+y+yRel]
			                              [mv->esel.x+x+xRel];
	
			node_copy(mTmp, nTmp, 0, mDst, nDst, mDst->nlayers-1);
		}
	}
	
	mv->esel.x += xRel;
	mv->esel.y += yRel;
}

void
nodesel_end_move(struct mapview *mv)
{
	struct map *mDst = mv->map;
	struct map *mTmp = &mv->esel.map;
	int x, y;

	for (y = 0; y < mv->esel.h; y++) {
		for (x = 0; x < mv->esel.w; x++) {
			struct node *node = &mDst->map[mv->esel.y+y]
			                              [mv->esel.x+x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer == mDst->nlayers-1)
					nref->layer = mDst->cur_layer;
			}
		}
	}
	
	map_pop_layer(mDst);
	
	map_reinit(mTmp);
	map_destroy(mTmp);
	mv->esel.moving = 0;
}

/* Copy the selection to the copy buffer. */
int
nodesel_copy(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;
	struct map *copybuf = &mapedit.copybuf;
	struct map *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to copy."));
		return (0);
	}
	if (copybuf->map != NULL) {
		map_free_nodes(copybuf);
	}
	if (map_alloc_nodes(copybuf, mv->esel.w, mv->esel.h) == -1) {
		text_msg(MSG_ERROR, "%s", error_get());
		return (0);
	}

	for (sy = mv->esel.y, dy = 0;
	     sy < mv->esel.y + mv->esel.h;
	     sy++, dy++) {
		for (sx = mv->esel.x, dx = 0;
		     sx < mv->esel.x + mv->esel.w;
		     sx++, dx++) {
			node_copy(m, &m->map[sy][sx], m->cur_layer, copybuf,
			    &copybuf->map[dy][dx], 0);
		}
	}
	return (1);
}

int
nodesel_paste(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;
	struct map *copybuf = &mapedit.copybuf;
	struct map *m = mv->map;
	int sx, sy, dx, dy;
	
	if (copybuf->map == NULL) {
		text_msg(MSG_ERROR, _("The copy buffer is empty!"));
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

	dprintf("[%dx%d] at [%d,%d]\n", copybuf->mapw, copybuf->maph, dx, dy);

	for (sy = 0, dy = mv->esel.y;
	     sy < copybuf->maph && dy < m->maph;
	     sy++, dy++) {
		for (sx = 0, dx = mv->esel.x;
		     sx < copybuf->mapw && dx < m->mapw;
		     sx++, dx++) {
			node_copy(copybuf, &copybuf->map[sy][sx], 0,
			    m, &m->map[dy][dx], m->cur_layer);
		}
	}
	return (1);
}

int
nodesel_kill(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	int x, y;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to kill."));
		return (0);
	}
	
	dprintf("[%d,%d]+[%d,%d]\n", mv->esel.x, mv->esel.y, mv->esel.w,
	    mv->esel.h);

	for (y = mv->esel.y; y < mv->esel.y + mv->esel.h; y++) {
		for (x = mv->esel.x; x < mv->esel.x + mv->esel.w; x++) {
			node_clear(m, &m->map[y][x], m->cur_layer);
		}
	}
	return (1);
}

int
nodesel_cut(struct tool *t, SDLKey key, int state, void *arg)
{
	struct mapview *mv = t->mv;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to cut."));
		return (0);
	}
	nodesel_copy(t, 0, 1, NULL);
	nodesel_kill(t, 0, 1, NULL);
	return (1);
}

static void
nodesel_init(struct tool *t)
{
	tool_bind_key(t, KMOD_CTRL, SDLK_c, nodesel_copy, NULL);
	tool_bind_key(t, KMOD_CTRL, SDLK_v, nodesel_paste, NULL);
	tool_bind_key(t, KMOD_CTRL, SDLK_x, nodesel_cut, NULL);
	tool_bind_key(t, KMOD_CTRL, SDLK_k, nodesel_kill, NULL);
}

const struct tool nodesel_tool = {
	N_("Node selection"),
	N_("Select and manipulate nodes."),
	SELECT_TOOL_ICON, SELECT_CURSORBMP,
	TOOL_HIDDEN,
	nodesel_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
