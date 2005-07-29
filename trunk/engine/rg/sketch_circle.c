/*	$Csoft: sketch_circle.c,v 1.2 2005/03/06 06:30:36 vedge Exp $	*/

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

struct circle_tool {
	struct tileview_tool tool;
	enum {
		BEGIN_CIRCLE,
		SET_RADIUS
	} seq;
	struct vg_element *cur_circle;
	struct vg_vertex *cur_radius;
};

static void
init(void *p)
{
	struct circle_tool *ct = p;

	ct->cur_circle = NULL;
	ct->cur_radius = NULL;
	ct->seq = BEGIN_CIRCLE;
}

static void
mousebuttondown(void *p, struct sketch *sk, double x, double y, int button)
{
	struct circle_tool *ct = p;
	struct vg *vg = sk->vg;
	struct tileview *tv = TILEVIEW_TOOL(ct)->tv;
	Uint8 r, g, b;

	switch (ct->seq) {
	case BEGIN_CIRCLE:
		if (button == SDL_BUTTON_LEFT) {
			ct->cur_circle = vg_begin_element(vg, VG_CIRCLE);
			vg_vertex2(vg, x, y);
			ct->cur_radius = vg_vertex2(vg, x, y);
			ct->seq = SET_RADIUS;
			tv->flags |= TILEVIEW_NO_SCROLLING;
	
			prim_hsv2rgb(sk->h, sk->s, sk->v, &r, &g, &b);
			vg_color4(vg, r, g, b, (int)(sk->a*255.0));
		}
		break;
	case SET_RADIUS:
		if (button == SDL_BUTTON_RIGHT) {
			vg_destroy_element(vg, ct->cur_circle);
		}
		ct->cur_radius = NULL;
		ct->cur_circle = NULL;
		ct->seq = BEGIN_CIRCLE;
		tv->flags &= ~TILEVIEW_NO_SCROLLING;
		break;
	}
}

static void
mousemotion(void *p, struct sketch *sk, double x, double y, double xrel,
    double yrel)
{
	struct circle_tool *ct = p;
	struct tileview *tv = TILEVIEW_TOOL(ct)->tv;

	if (ct->cur_circle != NULL) {
		ct->cur_radius->x = x;
		ct->cur_radius->y = y;
		ct->cur_circle->vg_circle.radius =
		    sqrt(pow(x - sk->vg->origin[2].x, 2) +
		         pow(y - sk->vg->origin[2].y, 2));
	} else {
		sk->vg->origin[2].x = x;
		sk->vg->origin[2].y = y;
	}
	sk->vg->redraw++;
}

struct tileview_sketch_tool_ops sketch_circle_ops = {
	{
		N_("Circle"),
		N_("Draw circles."),
		sizeof(struct circle_tool),
		TILEVIEW_SKETCH_TOOL,
		VGCIRCLES_ICON, -1,
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

