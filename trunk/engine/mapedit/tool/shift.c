/*	$Csoft: shift.c,v 1.22 2003/06/06 02:47:52 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include "shift.h"

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

const struct tool_ops shift_ops = {
	{
		NULL,		/* init */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	shift_window,
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL			/* mouse */
};

void
shift_init(void *p)
{
	struct shift *sh = p;

	tool_init(&sh->tool, "shift", &shift_ops);
	TOOL(sh)->icon = SPRITE(&mapedit, MAPEDIT_TOOL_SHIFT);
	sh->mode = 0;
	sh->multi = 0;
}

struct window *
shift_window(void *p)
{
	struct shift *sh = p;
	struct window *win;
	struct vbox *vb;

	win = window_new("mapedit-tool-shift");
	window_set_caption(win, _("Shift"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	vb = vbox_new(win, 0);
	{
		static const char *modes[] = {
			N_("Highest"),
			N_("All"),
			NULL
		};
		struct radio *rad;
		struct checkbox *cb;

		rad = radio_new(vb, modes);
		widget_bind(rad, "value", WIDGET_INT, NULL, &sh->mode);
		
		cb = checkbox_new(vb, _("Multi"));
		widget_bind(cb, "state", WIDGET_INT, NULL, &sh->multi);
	}
	return (win);
}

void
shift_mouse(void *p, struct mapview *mv, Sint16 relx, Sint16 rely)
{
	struct shift *sh = p;
	int selx = mv->mx + mv->mouse.x;
	int sely = mv->my + mv->mouse.y;
	int w = 1;
	int h = 1;
	int x, y;

	if (!sh->multi ||
	    mapview_get_selection(mv, &selx, &sely, &w, &h) == -1) {
		if (selx < 0 || selx >= mv->map->mapw ||
		    sely < 0 || sely >= mv->map->maph)
			return;
	}

	for (y = sely; y < sely+h; y++) {
		for (x = selx; x < selx+w; x++) {
			struct node *node = &mv->map->map[y][x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != mv->map->cur_layer)
					continue;

//				if (SDL_GetModState() & KMOD_CTRL) {
					noderef_set_center(nref,
					    nref->xcenter+relx,
					    nref->ycenter+rely);
#if 0
				} else {
					noderef_set_motion(nref,
					    nref->xmotion+relx,
					    nref->ymotion+rely);

					dprintf("motion: %d,%d\n",
					    (int)nref->xmotion,
					    (int)nref->ymotion);

					if (nref->xmotion < -(TILEW/2)) {
						if (node_move_ref(nref, node,
						    mv->map, x-1, y) == 0)
							mv->mouse.x--;
						nref->xmotion = TILEW/2;
					} else if (nref->xmotion > TILEW/2) {
						if (node_move_ref(nref, node,
						    mv->map, x+1, y) == 0)
							mv->mouse.x++;
						nref->xmotion = -(TILEW/2);
					}
					if (nref->ymotion < -(TILEH/2)) {
						if (node_move_ref(nref, node,
						    mv->map, x, y-1) == 0)
							mv->mouse.y--;
						nref->ymotion = TILEH/2;
					} else if (nref->ymotion > TILEH/2) {
						if (node_move_ref(nref, node,
						    mv->map, x, y+1) == 0)
							mv->mouse.y++;
						nref->ymotion = -(TILEH/2);
					}
				}
#endif
				if (sh->mode == SHIFT_HIGHEST)
					break;
			}
		}
	}
}

