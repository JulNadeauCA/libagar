/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/radio.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/fspinbutton.h>

#include "tileset.h"
#include "tileview.h"

struct sketch_line_tool {
	struct tileview_tool tool;
	int seq;
	struct vg_element *cur_line;
	struct vg_vertex *cur_vtx;
	int mode;
};

static void
init(void *p)
{
	struct sketch_line_tool *lt = p;

	lt->seq = 0;
	lt->cur_line = NULL;
	lt->cur_vtx = NULL;
	lt->mode = 1;		/* Line strip */
}

static struct window *
edit(void *p)
{
	struct sketch_line_tool *lt = p;
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	win = window_new(0, NULL);
	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &lt->mode);
	return (win);
}

struct tileview_sketch_tool_ops sketch_line_ops = {
	{
		_("Lines"),
		_("Line segments, strips and loops."),
		sizeof(struct sketch_line_tool),
		TILEVIEW_SKETCH_TOOL,
		VGLINES_ICON, -1,
		init,
		NULL,		/* destroy */
		edit,
		NULL,		/* keydown */
		NULL		/* keyup */
	},
	NULL,		/* mousebuttondown */
	NULL,		/* mousebuttonup */
	NULL		/* mousemotion */
};

