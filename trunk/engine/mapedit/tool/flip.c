/*	$Csoft: flip.c,v 1.17 2003/09/07 04:17:37 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include "flip.h"

#include <engine/widget/radio.h>

static void	flip_effect(void *, struct mapview *, struct map *,
		            struct node *);

const struct tool_ops flip_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* cursor */
	flip_effect,
	NULL			/* mouse */
};

void
flip_init(void *p)
{
	static const char *mode_items[] = {
		N_("Horizontal"),
		N_("Vertical"),
		NULL
	};
	static const char *which_items[] = {
		N_("All"),
		N_("Highest"),
		NULL
	};
	struct flip *flip = p;
	struct window *win;
	struct radio *rad;

	tool_init(&flip->tool, "flip", &flip_ops, MAPEDIT_TOOL_FLIP);
	flip->mode = FLIP_HORIZ;
	flip->which = FLIP_ALL;
	
	TOOL(flip)->win = win = window_new("mapedit-tool-flip");
	window_set_caption(win, _("Flip"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	event_new(win, "window-close", tool_window_close, "%p", flip);

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &flip->mode);

	rad = radio_new(win, which_items);
	widget_bind(rad, "value", WIDGET_INT, &flip->which);
}

static void
flip_effect(void *p, struct mapview *mv, struct map *m, struct node *node)
{
	struct flip *flip = p;
	struct noderef *nref;
	
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		enum transform_type type = TRANSFORM_HFLIP;
		struct transform *trans;
		
		if (nref->layer != m->cur_layer)
			continue;

		switch (flip->mode) {
		case FLIP_HORIZ:
			type = TRANSFORM_HFLIP;
			dprintf("horiz\n");
			break;
		case FLIP_VERT:
			type = TRANSFORM_VFLIP;
			dprintf("vert\n");
			break;
		}

		/* Same transform already applied? */
		TAILQ_FOREACH(trans, &nref->transforms, transforms) {
			if (trans->type == type) {
				TAILQ_REMOVE(&nref->transforms, trans,
				    transforms);
				dprintf("removed duplicate\n");
				break;
			}
		}
		if (trans != NULL) {				/* Existing */
			dprintf("existing transform %d\n", type);
			continue;
		}

		if ((trans = transform_new(type, 0, NULL)) == NULL) {
			text_msg(MSG_ERROR, "%s", error_get());
			continue;
		}
		TAILQ_INSERT_TAIL(&nref->transforms, trans, transforms);

		if (flip->which == FLIP_HIGHEST)
			break;
	}
}

