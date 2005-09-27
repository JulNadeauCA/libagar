/*	$Csoft: sketch_line.c,v 1.5 2005/07/29 06:33:41 vedge Exp $	*/

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
	RG_TileviewTool tool;
	enum {
		BEGIN_LINE,
		CONTINUE_LINE
	} seq;
	VG_Element *cur_line;
	VG_Vtx *cur_vtx;
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

static AG_Window *
edit(void *p)
{
	struct line_tool *lt = p;
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	AG_Window *win;
	AG_Radio *rad;

	win = AG_WindowNew(0, NULL);
	rad = AG_RadioNew(win, mode_items);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &lt->mode);
	return (win);
}

static void
mousebuttondown(void *p, RG_Sketch *sk, double x, double y, int button)
{
	struct line_tool *lt = p;
	VG *vg = sk->vg;
	RG_Tileview *tv = RG_TILEVIEW_TOOL(lt)->tv;
	Uint8 r, g, b;

	switch (lt->seq) {
	case BEGIN_LINE:
		if (button == SDL_BUTTON_LEFT) {
			lt->cur_line = VG_Begin(vg,
			    lt->mode == 1 ? VG_LINE_STRIP :
			    lt->mode == 2 ? VG_LINE_LOOP :
			    VG_LINES);
			VG_Vertex2(vg, x, y);
			lt->cur_vtx = VG_Vertex2(vg, x, y);
			lt->seq = CONTINUE_LINE;
			tv->flags |= RG_TILEVIEW_NO_SCROLLING;
	
			RG_HSV2RGB(sk->h, sk->s, sk->v, &r, &g, &b);
			VG_Color4(vg, r, g, b, (int)(sk->a*255.0));
		}
		break;
	case CONTINUE_LINE:
		if (button == SDL_BUTTON_LEFT) {
			lt->cur_vtx = VG_Vertex2(vg, x, y);
		} else {
			if (lt->cur_line->nvtx <= 2) {
				VG_DestroyElement(vg, lt->cur_line);
			} else {
				VG_PopVertex(vg);
			}
			lt->cur_vtx = NULL;
			lt->cur_line = NULL;
			lt->seq = BEGIN_LINE;
			tv->flags &= ~RG_TILEVIEW_NO_SCROLLING;
		}
		break;
	}
}

static void
mousemotion(void *p, RG_Sketch *sk, double x, double y, double xrel,
    double yrel)
{
	struct line_tool *lt = p;
	RG_Tileview *tv = RG_TILEVIEW_TOOL(lt)->tv;

	if (lt->cur_line != NULL) {
		lt->cur_vtx->x = x;
		lt->cur_vtx->y = y;
		sk->vg->redraw++;
	}
}

RG_TileviewSketchToolOps sketch_line_ops = {
	{
		N_("Lines"),
		N_("Line segments, strips and loops."),
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

