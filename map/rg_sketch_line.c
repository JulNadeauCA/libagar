/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>

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
init(void *_Nonnull p)
{
	struct line_tool *lt = p;

	lt->mode = 1;
	lt->cur_line = NULL;
	lt->cur_vtx = NULL;
	lt->seq = BEGIN_LINE;
}

static AG_Window *_Nonnull
edit(void *_Nonnull p)
{
	struct line_tool *lt = p;
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	AG_Window *win;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_RadioNewInt(win, AG_RADIO_HFILL, mode_items, &lt->mode);
	return (win);
}

static void
mousebuttondown(void *_Nonnull p, RG_Sketch *_Nonnull sk, float x, float y,
    int button)
{
	struct line_tool *lt = p;
	VG *vg = sk->vg;
	RG_Tileview *tv = RG_TILEVIEW_TOOL(lt)->tv;
	Uint8 r, g, b;

	switch (lt->seq) {
	case BEGIN_LINE:
		if (button == AG_MOUSE_LEFT) {
			lt->cur_line = VG_Begin(vg,
			    lt->mode == 1 ? VG_LINE_STRIP :
			    lt->mode == 2 ? VG_LINE_LOOP :
			    VG_LINES);
			VG_Vertex2(vg, x, y);
			lt->cur_vtx = VG_Vertex2(vg, x, y);
			lt->seq = CONTINUE_LINE;
			tv->flags |= RG_TILEVIEW_NO_SCROLLING;
	
			AG_HSV2RGB(sk->h, sk->s, sk->v, &r, &g, &b);
			VG_Color4(vg, r, g, b, (int)(sk->a*255.0));
		}
		break;
	case CONTINUE_LINE:
		if (button == AG_MOUSE_LEFT) {
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
mousemotion(void *_Nonnull p, RG_Sketch *_Nonnull sk, float x, float y,
    float xrel, float yrel)
{
	struct line_tool *lt = p;

	if (lt->cur_line != NULL) {
		lt->cur_vtx->x = x;
		lt->cur_vtx->y = y;
	}
}

RG_TileviewSketchToolOps sketch_line_ops = {
	{
		N_("Lines"),
		N_("Line segments, strips and loops."),
		sizeof(struct line_tool),
		TILEVIEW_SKETCH_TOOL,
		NULL, -1,
		AG_KEY_L,
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
