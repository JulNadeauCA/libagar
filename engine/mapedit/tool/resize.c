/*	$Csoft: resize.c,v 1.37 2004/03/30 15:56:53 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

static void resize_mousemotion(struct tool *, int, int, int, int, int, int,
                               int, int, int);

const struct tool resize_tool = {
	N_("Resize tool"),
	N_("Resize the node array."),
	RESIZE_TOOL_ICON,
	-1,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	resize_mousemotion,
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

static void
resize_mousemotion(struct tool *t, int x, int y, int xrel, int yrel, int xo,
    int yo, int xorel, int yorel, int b)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	int w = m->mapw;
	int h = m->maph;

	if ((b & SDL_BUTTON(1)) == 0)
		return;

	if (xrel < 0 && --w < 1) {
		w = 1;
	} else if (xrel > 0 && ++w > MAP_MAX_WIDTH) {
		w = MAP_MAX_WIDTH;
	}
	
	if (yrel < 0 && --h < 1) {
		h = 1;
	} else if (yrel > 0 && ++h > MAP_MAX_HEIGHT) {
		h = MAP_MAX_HEIGHT;
	}

	if (w < 1) {
		w = 1;
	} else if (w > MAP_MAX_WIDTH) {
		w = MAP_MAX_WIDTH;
	}

	if (h < 1) {
		h = 1;
	} else if (h > MAP_MAX_HEIGHT) {
		h = MAP_MAX_HEIGHT;
	}

	if (w != m->mapw || h != m->maph) {
		mv->esel.set = 0;
		map_resize(m, w, h);
	}
}
