/*	$Csoft: resize.c,v 1.24 2003/03/25 13:48:05 vedge Exp $	*/

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

#include "resize.h"

static const struct tool_ops resize_ops = {
	{
		tool_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	NULL,			/* window */
	NULL,			/* resize */
	NULL,			/* effect */
	resize_mouse
};

void
resize_init(void *p)
{
	struct resize *res = p;

	tool_init(&res->tool, "resize", &resize_ops);
	res->mode = RESIZE_GROW;
}

void
resize_mouse(void *p, struct mapview *mv, Sint16 xrel, Sint16 yrel, Uint8 state)
{
	struct map *m = mv->map;

	if (mv->cxrel < 0 && m->mapw > 1) {
		map_resize(m, m->mapw - 1, m->maph);
	} else if (mv->cxrel > 0) {
		map_resize(m, m->mapw + 1, m->maph);
	}
	if (mv->cyrel < 0 && m->maph > 1) {
		map_resize(m, m->mapw, m->maph - 1);
	} else if (mv->cyrel > 0) {
		map_resize(m, m->mapw, m->maph + 1);
	}
}
