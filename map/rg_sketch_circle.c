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
#include <agar/map/rg_math.h>

struct circle_tool {
	RG_TileviewTool tool;
	enum {
		BEGIN_CIRCLE,
		SET_RADIUS
	} seq;
	VG_Element *cur_circle;
	VG_Vtx *cur_radius;
};

static void
init(void *_Nonnull p)
{
	struct circle_tool *ct = p;

	ct->cur_circle = NULL;
	ct->cur_radius = NULL;
	ct->seq = BEGIN_CIRCLE;
}

static void
mousebuttondown(void *_Nonnull p, RG_Sketch *_Nonnull sk, float x, float y,
    int button)
{
	struct circle_tool *ct = p;
	VG *vg = sk->vg;
	RG_Tileview *tv = RG_TILEVIEW_TOOL(ct)->tv;
	Uint8 r, g, b;

	switch (ct->seq) {
	case BEGIN_CIRCLE:
		if (button == AG_MOUSE_LEFT) {
			ct->cur_circle = VG_Begin(vg, VG_CIRCLE);
			VG_Vertex2(vg, x, y);
			ct->cur_radius = VG_Vertex2(vg, x, y);
			ct->seq = SET_RADIUS;
			tv->flags |= RG_TILEVIEW_NO_SCROLLING;
	
			AG_HSV2RGB(sk->h, sk->s, sk->v, &r, &g, &b);
			VG_Color4(vg, r, g, b, (int)(sk->a*255.0));
		}
		break;
	case SET_RADIUS:
		if (button == AG_MOUSE_RIGHT) {
			VG_DestroyElement(vg, ct->cur_circle);
		}
		ct->cur_radius = NULL;
		ct->cur_circle = NULL;
		ct->seq = BEGIN_CIRCLE;
		tv->flags &= ~RG_TILEVIEW_NO_SCROLLING;
		break;
	}
}

static void
mousemotion(void *_Nonnull p, RG_Sketch *_Nonnull sk, float x, float y,
    float xrel, float yrel)
{
	struct circle_tool *ct = p;

	if (ct->cur_circle != NULL) {
		ct->cur_radius->x = x;
		ct->cur_radius->y = y;
		ct->cur_circle->vg_circle.radius =
		    Hypot(x - sk->vg->origin[2].x,
		          y - sk->vg->origin[2].y);
	} else {
		sk->vg->origin[2].x = x;
		sk->vg->origin[2].y = y;
	}
}

RG_TileviewSketchToolOps sketch_circle_ops = {
	{
		N_("Circle"),
		N_("Draw circles."),
		sizeof(struct circle_tool),
		TILEVIEW_SKETCH_TOOL,
		NULL, -1,
		AG_KEY_C,
		init,
		NULL,		/* destroy */
		NULL,		/* edit */
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
