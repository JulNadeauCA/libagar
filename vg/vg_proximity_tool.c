/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Generate plots of proximity functions for debugging purposes.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector v, int button)
{
	VG_Tool *t = obj;
	VG_View *vv = t->vgv;
	VG_Node *vn;

	if (button == AG_MOUSE_LEFT) {
		if ((vn = VG_Nearest(vv, v)) == NULL) {
			return (0);
		}
		if (AG_GetModState(vv) & AG_KEYMOD_CTRL) {
			AG_INVFLAGS(vn->flags, VG_NODE_SELECTED);
		} else {
			VG_UnselectAll(vv->vg);
			vn->flags |= VG_NODE_SELECTED;
		}
		return (1);
	}
	return (0);
}

static void
PostDraw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Node *vn;
	AG_Rect r;
	VG_Vector v;
	VG_Color c;
	float prox, vRange = 80.0f;
	Uint w = WIDGET(vv)->w;
	Uint h = WIDGET(vv)->h;

	r.w = 5;
	r.h = 5;
	for (r.y = 0; r.y < h; r.y += r.h-1) {
		for (r.x = 0; r.x < w; r.x+=r.w-1) {
			prox = AG_FLT_MAX;
			TAILQ_FOREACH(vn, &vv->vg->nodes, list) {
				if ((vn->flags & VG_NODE_SELECTED) == 0 ||
				    vn->ops->pointProximity == NULL) {
					continue;
				}
				VG_GetVGCoordsFlt(vv,
				    VGVECTOR(r.x + (r.w >> 1),
				             r.y + (r.h >> 1)), &v);
				prox = vn->ops->pointProximity(vn, vv, &v);
				break;
			}
			if (prox < vRange) {
				AG_Color ac;

				if (prox == 0.0f) {
					c.r = 200;
					c.g = 200;
					c.b = 0;
				} else {
					c.r = 0;
					c.g = 255 - (Uint8)(prox*255.0f/vRange);
					c.b = 0;
				}
				c.a = 255;
				ac = VG_MapColorRGB(c);
				AG_DrawRectFilled(vv, &r, &ac);
			}
		}
	}
}

VG_ToolOps vgProximityTool = {
	N_("Proximity"),
	N_("Plot proximity functions of selected entities."),
	&vgIconProximity,
	sizeof(VG_Tool),
	VG_NOSNAP,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* predraw */
	PostDraw,
	NULL,			/* selected */
	NULL,			/* deselected */
	NULL,			/* mousemotion */
	MouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
