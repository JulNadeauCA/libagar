/*	$Csoft: select.c,v 1.8 2003/03/25 13:48:05 vedge Exp $	*/

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

#include "select.h"

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/mapedit/selops.h>

static const struct tool_ops select_ops = {
	{
		tool_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	NULL,			/* window */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL			/* mouse */
};

static void
select_copy(void *p, struct mapview *mv)
{
	if (mv->esel.set)
		selops_copy(mv);
}

static void
select_paste(void *p, struct mapview *mv)
{
	if (mv->esel.set) {
		selops_paste(mv, mv->esel.x, mv->esel.y);
	} else {
		if (mv->cx != -1 && mv->cy != -1) {
			selops_paste(mv, mv->cx, mv->cy);
		} else {
			selops_paste(mv, 0, 0);
		}
	}
}

static void
select_cut(void *p, struct mapview *mv)
{
	if (mv->esel.set)
		selops_cut(mv);
}

static void
select_kill(void *p, struct mapview *mv)
{
	if (mv->esel.set)
		selops_kill(mv);
}

void
select_init(void *p)
{
	struct select *sel = p;

	tool_init(&sel->tool, "select", &select_ops);
	tool_bind_key(sel, KMOD_CTRL, SDLK_c, select_copy, 0);
	tool_bind_key(sel, KMOD_CTRL, SDLK_v, select_paste, 1);
	tool_bind_key(sel, KMOD_CTRL, SDLK_x, select_cut, 1);
	tool_bind_key(sel, KMOD_CTRL, SDLK_k, select_kill, 1);
}

