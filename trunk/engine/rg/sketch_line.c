/*	$Csoft: sketch_line.c,v 1.1 2005/03/03 10:51:01 vedge Exp $	*/

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
#include <engine/widget/label.h>

#include "tileset.h"
#include "tileview.h"

struct line_tool {
	struct tileview_tool tool;
	enum {
		BEGIN_LINE,
		CONTINUE_LINE
	} seq;
	struct vg_element *cur_line;
	struct vg_vertex *cur_vtx;
	int mode;
};

static void
init(void *p)
{
	struct line_tool *lt = p;

	lt->mode = 1;
	lt->cur_line = NULL;
	lt->cur_vtx = NULL;
	lt->seq = BEGIN_LINE;
}

static struct window *
edit(void *p)
{
	struct line_tool *lt = p;
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
#ifdef DEBUG
	label_new(win, LABEL_POLLED, "Line: %p\nVertex: %p\n", &lt->cur_line,
	    &lt->cur_vtx);
#endif
	return (win);
}

static void
mousebuttondown(void *p, struct sketch *sk, double x, double y, int button)
{
	struct line_tool *lt = p;
	struct vg *vg = sk->vg;
	struct tileview *tv = TILEVIEW_TOOL(lt)->tv;
	Uint8 r, g, b;

	switch (lt->seq) {
	case BEGIN_LINE:
		if (button == SDL_BUTTON_LEFT) {
			dprintf("begin line %.2f,%.2f\n", x, y);
			lt->cur_line = vg_begin_element(vg, VG_LINE_STRIP);
			vg_vertex2(vg, x, y);
			lt->cur_vtx = vg_vertex2(vg, x, y);
			lt->seq = CONTINUE_LINE;
	
			prim_hsv2rgb(sk->h/360.0, sk->s, sk->v, &r, &g, &b);
			vg_color4(vg, r, g, b, (int)(sk->a*255.0));
		}
		break;
	case CONTINUE_LINE:
		if (button == SDL_BUTTON_LEFT) {
			dprintf("cont line\n");
			lt->cur_vtx = vg_vertex2(vg, x, y);
			lt->cur_line->redraw = 1;
		} else {
			dprintf("abort line\n");
			lt->seq = BEGIN_LINE;
		}
		lt->seq = BEGIN_LINE;
		lt->cur_vtx = NULL;
		lt->cur_line = NULL;
		break;
	}
	vg_redraw_elements(vg);
	vg_rasterize(vg);
	tv->tile->flags |= TILE_DIRTY;
}

static void
mousemotion(void *p, struct sketch *sk, double x, double y, double xrel,
    double yrel)
{
	struct line_tool *lt = p;
	struct tileview *tv = TILEVIEW_TOOL(lt)->tv;

	if (lt->cur_line != NULL) {
		dprintf("move vtx %.2f,%.2f\n", x, y);
		lt->cur_vtx->x = x;
		lt->cur_vtx->y = y;
		lt->cur_line->redraw = 1;
	}
	vg_rasterize(sk->vg);
	tv->tile->flags |= TILE_DIRTY;
}

struct tileview_sketch_tool_ops sketch_line_ops = {
	{
		_("Lines"),
		_("Line segments, strips and loops."),
		sizeof(struct line_tool),
		TILEVIEW_SKETCH_TOOL,
		VGLINES_ICON, -1,
		init,
		NULL,		/* destroy */
		edit,
		NULL,		/* selected */
		NULL		/* unselected */
	},
	mousebuttondown,
	NULL,			/* mousebuttonup */
	mousemotion,
	NULL,			/* mousewheel */
	NULL,			/* keydown */
	NULL,			/* keyup */
};

