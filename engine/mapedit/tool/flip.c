/*	$Csoft: flip.c,v 1.7 2003/03/28 00:23:23 vedge Exp $	*/

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
#include <engine/widget/text.h>

static const struct tool_ops flip_ops = {
	{
		tool_destroy,
		NULL,		/* load */
		NULL		/* save */
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
	flip->mode = FLIP_HORIZ;
	flip->which = FLIP_ALL;
}

struct window *
flip_window(void *p)
{
	struct flip *flip = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-flip", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    177, 196,
	    177, 196);
	window_set_caption(win, "Flip");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		static const char *mode_items[] = {
			"Horizontal",
			"Vertical",
			NULL
		};
		static const char *which_items[] = {
			"All",
			"Highest",
			NULL
		};
		struct radio *rad;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &flip->mode);
		win->focus = WIDGET(rad);
		
		rad = radio_new(reg, which_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &flip->which);
	}
	return (win);
}

void
flip_effect(void *p, struct mapview *mv, struct node *node)
{
	struct flip *flip = p;
	struct noderef *nref;
	
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		enum transform_type type = TRANSFORM_HFLIP;
		struct transform *trans;
		
		if (nref->layer != mv->map->cur_layer)
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
			text_msg("Error initing transform", "%s", error_get());
			continue;
		}
		SLIST_INSERT_HEAD(&nref->transforms, trans, transforms);

		if (flip->which == FLIP_HIGHEST)
			break;
	}
}

