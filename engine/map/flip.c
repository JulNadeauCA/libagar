/*	$Csoft: flip.c,v 1.2 2005/04/16 05:52:27 vedge Exp $	*/

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

#include "map.h"
#include "mapedit.h"

static int multi_ind = 0;		/* Apply to tiles individually */

static void
flip_init(struct tool *t)
{
	static const char *mode_items[] = {
		N_("Horizontal"),
		N_("Vertical"),
		NULL
	};
	struct window *win;
	struct radio *rad;
	struct checkbox *cb;

	win = tool_window(t, "mapedit-tool-flip");

	cb = checkbox_new(win, _("Flip entire selection"));
	widget_bind(cb, "state", WIDGET_INT, &multi_ind);
	
	tool_push_status(t, _("Specify entity/selection (L=mirror, R=flip)."));
}

static void
toggle_transform(struct noderef *nref, int type)
{
	struct transform *trans;

	TAILQ_FOREACH(trans, &nref->transforms, transforms) {
		if (trans->type == type) {
			TAILQ_REMOVE(&nref->transforms, trans, transforms);
			return;
		}
	}
	if ((trans = transform_new(type, 0, NULL)) == NULL) {
		text_msg(MSG_ERROR, "%s", error_get());
		return;
	}
	TAILQ_INSERT_TAIL(&nref->transforms, trans, transforms);
}

#if 0
static void
flip_mousebuttondown(struct tool *t, int mx, int my, int xoff, int yoff, int b)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	int selx = mv->mx + mv->mouse.x;
	int sely = mv->my + mv->mouse.y;
	int w = 1;
	int h = 1;
	int x, y;
	int mode = (b == SDL_BUTTON_LEFT) ? TRANSFORM_HFLIP : TRANSFORM_VFLIP;

	if (!multi_ind ||
	    mapview_get_selection(mv, &selx, &sely, &w, &h) == -1) {
		if (selx < 0 || selx >= m->mapw ||
		    sely < 0 || sely >= m->maph)
			return;
	}

	for (y = sely; y < sely+h; y++) {
		for (x = selx; x < selx+w; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != m->cur_layer)
					continue;

				toggle_transform(nref, mode);
			}
		}
	}
}
#else

static void
flip_mousebuttondown(struct tool *t, int mx, int my, int xoff, int yoff, int b)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	int selx = mv->mx + mv->mouse.x;
	int sely = mv->my + mv->mouse.y;
	int w = 1;
	int h = 1;
	int x, y;

	if (!multi_ind ||
	    mapview_get_selection(mv, &selx, &sely, &w, &h) == -1) {
		if (selx < 0 || selx >= m->mapw ||
		    sely < 0 || sely >= m->maph)
			return;
	}

	for (y = sely; y < sely+h; y++) {
		for (x = selx; x < selx+w; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != m->cur_layer)
					continue;

				if (b == SDL_BUTTON_LEFT) {
					toggle_transform(nref, TRANSFORM_HFLIP);
				} else if (b == SDL_BUTTON_RIGHT) {
					toggle_transform(nref, TRANSFORM_VFLIP);
				}
			}
		}
	}
}

#endif

static int
flip_cursor(struct tool *t, SDL_Rect *rd)
{
	return (-1);
}

const struct tool flip_tool = {
	N_("Flip Tile"),
	N_("Apply a flip/mirror transformation on a tile."),
	FLIP_TOOL_ICON,
	FLIP_TOOL_ICON,
	flip_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	flip_cursor,
	NULL,			/* effect */
	NULL,			/* mousemotion */
	flip_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

#endif /* MAP */
