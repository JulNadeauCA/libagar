/*	$Csoft: flip.c,v 1.2 2003/03/16 04:00:37 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/version.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include <libfobj/fobj.h>

#include "tool.h"
#include "flip.h"

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
}

struct window *
flip_window(void *p)
{
	struct flip *flip = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-flip", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    157, 76,
	    157, 76);
	window_set_caption(win, "Flip");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		static const char *mode_items[] = {
			"Horizontal",
			"Vertical",
			NULL
		};
		struct radio *rad;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &flip->mode);
		win->focus = WIDGET(rad);
	}
	return (win);
}

void
flip_effect(void *p, struct mapview *mv, struct node *node)
{
	struct flip *flip = p;
	struct map *m = mv->map;
	struct transform *trans;
	enum transform_type type;
	struct noderef *nref;
	
	trans = emalloc(sizeof(struct transform));
	type = (flip->mode == FLIP_HORIZ) ? TRANSFORM_HFLIP : TRANSFORM_VFLIP;
	if (transform_init(trans, type, 0, NULL) == -1) {
		text_msg("Error initing transform", "%s", error_get());
		free(trans);
		return;
	}

	/* XXX remove other flips! */
	/* XXX highest, all */

	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		if (nref->layer != mv->cur_layer)
			continue;
		SLIST_INSERT_HEAD(&nref->transforms, trans, transforms);
	}
}

