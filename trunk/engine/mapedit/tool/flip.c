/*	$Csoft: flip.c,v 1.13 2003/06/29 11:33:45 vedge Exp $	*/

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

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>

const struct tool_ops flip_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	flip_window,
	NULL,
	flip_effect,
	NULL			/* mouse */
};

void
flip_init(void *p)
{
	struct flip *flip = p;

	tool_init(&flip->tool, "flip", &flip_ops);
	TOOL(flip)->icon = SPRITE(&mapedit, MAPEDIT_TOOL_FLIP);
	flip->mode = FLIP_HORIZ;
	flip->which = FLIP_ALL;
}

struct window *
flip_window(void *p)
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
	struct vbox *vb;
	struct radio *rad;

	win = window_new("mapedit-tool-flip");
	window_set_caption(win, _("Flip"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	vb = vbox_new(win, 0);
	{
		rad = radio_new(vb, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &flip->mode);
		rad = radio_new(vb, which_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &flip->which);
	}
	return (win);
}

void
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
		SLIST_FOREACH(trans, &nref->transforms, transforms) {
			if (trans->type == type) {
				SLIST_REMOVE(&nref->transforms, trans,
				    transform, transforms);
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
		SLIST_INSERT_HEAD(&nref->transforms, trans, transforms);

		if (flip->which == FLIP_HIGHEST)
			break;
	}
}

