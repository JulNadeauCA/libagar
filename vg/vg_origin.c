/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Origin element.
 */

#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"

int
VG_AddOrigin(VG *vg, float x, float y, float radius, VG_Color color)
{
	int o;

	vg->origin = Realloc(vg->origin,
	    (vg->norigin+1)*sizeof(VG_Vtx));
	vg->originRadius = Realloc(vg->originRadius,
	    (vg->norigin+1)*sizeof(float));
	vg->originColor = Realloc(vg->originColor,
	    (vg->norigin+1)*sizeof(Uint32));

	o = vg->norigin++;
	vg->origin[o].x = x;
	vg->origin[o].y = y;
	vg->originRadius[o] = radius;
	vg->originColor[o] = color;
	return (o);
}

void
VG_Origin(VG *vg, int o, float ox, float oy)
{
	VG_Element *vge;
	VG_Block *vgb;
	int i;

	if (o == 0) {
		if (vg->cur_block != NULL) {
			TAILQ_FOREACH(vge, &vg->cur_block->vges, vgbmbs) {
				for (i = 0; i < vge->nvtx; i++) {
					VG_Vtx *vtx = &vge->vtx[i];

					vtx->x -= ox - vg->origin[0].x;
					vtx->y -= oy - vg->origin[0].y;
				}
			}
			vg->cur_block->origin.x = ox;
			vg->cur_block->origin.y = oy;
			return;
		}

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			for (i = 0; i < vge->nvtx; i++) {
				VG_Vtx *vtx = &vge->vtx[i];

				vtx->x -= ox - vg->origin[0].x;
				vtx->y -= oy - vg->origin[0].y;
			}
		}
		TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
			vgb->pos.x -= ox - vg->origin[0].x;
			vgb->pos.y -= oy - vg->origin[0].y;
		}
		for (i = 1; i < vg->norigin; i++) {
			vg->origin[i].x -= ox - vg->origin[0].x;
			vg->origin[i].y -= oy - vg->origin[0].y;
		}
		vg->origin[0].x = ox;
		vg->origin[0].y = oy;
	} else {
		vg->origin[o].x = ox - vg->origin[0].x;
		vg->origin[o].y = oy - vg->origin[0].y;
	}
}

void
VG_OriginColor(VG *vg, int o, int r, int g, int b)
{
	vg->originColor[o] = VG_GetColorRGB(r,g,b);
}

void
VG_OriginRadius(VG *vg, int o, float r)
{
	vg->originRadius[o] = r;
}

void
VG_DrawOrigin(VG_View *vv)
{
	VG *vg = vv->vg;
	int x, y, rPixels;
	Uint32 c32;
	float r;
	int o;

	for (o = 0; o < vg->norigin; o++) {
		if (o == 0) {
			x = vg->origin[o].x*vv->scale;
			y = vg->origin[o].y*vv->scale;
		} else {
			VG_GetViewCoords(vv, vg->origin[o].x, vg->origin[o].y,
			    &x, &y);
		}
		r = vg->originRadius[o];
		c32 = VG_MapColorRGB(vg->originColor[o]);

		rPixels = (int)(r*vv->scale);
		AG_DrawCircle(vv, x, y, r, c32);
		AG_DrawLineH(vv, x, x+rPixels, y, c32);
		AG_DrawLineV(vv, x, y, y+rPixels, c32);
	}
}
