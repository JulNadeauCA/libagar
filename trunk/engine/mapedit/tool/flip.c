/*	$Csoft: flip.c,v 1.19 2003/12/05 01:21:26 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapedit.h>

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

static void flip_init(void);
static void flip_effect(struct mapview *, struct map *, struct node *);

struct tool flip_tool = {
	N_("Flip Tile"),
	N_("Apply a flip/mirror transformation on a tile."),
	MAPEDIT_TOOL_FLIP,
	-1,
	flip_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	flip_effect,
	NULL,			/* cursor */
	NULL			/* mouse */
};

static enum flip_mode {
	FLIP_HORIZ,		/* Mirror */
	FLIP_VERT		/* Flip */
} mode = FLIP_HORIZ;

static int multi = 0;			/* Flip/mirror whole selection */
static int multi_ind = 0;		/* Apply to tiles individually */

static void
flip_init(void)
{
	static const char *mode_items[] = {
		N_("Horizontal"),
		N_("Vertical"),
		NULL
	};
	struct window *win;
	struct radio *rad;
	struct checkbox *cb;

	win = tool_window_new(&flip_tool, "mapedit-tool-flip");

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &mode);
	
	cb = checkbox_new(win, _("Multi"));
	widget_bind(cb, "state", WIDGET_INT, &multi);

	cb = checkbox_new(win, _("Multi (individual)"));
	widget_bind(cb, "state", WIDGET_INT, &multi_ind);
}

static void
flip_effect(struct mapview *mv, struct map *m, struct node *node)
{
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
				enum transform_type type = TRANSFORM_HFLIP;
				struct transform *trans;
		
				if (nref->layer != m->cur_layer)
					continue;

				switch (mode) {
				case FLIP_HORIZ:
					type = TRANSFORM_HFLIP;
					break;
				case FLIP_VERT:
					type = TRANSFORM_VFLIP;
					break;
				}

				TAILQ_FOREACH(trans, &nref->transforms,
				    transforms) {
					if (trans->type == type) {
						TAILQ_REMOVE(&nref->transforms,
						    trans, transforms);
						break;
					}
				}
				if (trans != NULL)
					continue;

				if ((trans = transform_new(type, 0, NULL))
				    == NULL) {
					text_msg(MSG_ERROR, "%s", error_get());
					continue;
				}
				TAILQ_INSERT_TAIL(&nref->transforms, trans,
				    transforms);
				break;
			}
		}
	}
}

