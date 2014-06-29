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
 * Point element.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/numerical.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

static void
Init(void *p)
{
	VG_Point *pt = p;

	pt->size = 0.0f;
}

static void
Draw(void *p, VG_View *vv)
{
	VG_Point *pt = p;
	float size, i;

	if (vv->flags & VG_VIEW_CONSTRUCTION) {
		size = 3.0f;
	} else {
		size = pt->size;
	}
	if (size > 0.0f) {
		AG_Color c = VG_MapColorRGB(VGNODE(pt)->color);
		int x, y;

		VG_GetViewCoords(vv, VG_Pos(pt), &x, &y);
		AG_PutPixel(vv, x, y, c);
		for (i = 0; i < size; i += 1.0f) {
			AG_PutPixel(vv, x-i, y, c);
			AG_PutPixel(vv, x+i, y, c);
			AG_PutPixel(vv, x, y-i, c);
			AG_PutPixel(vv, x, y+i, c);
		}
	}
}

static void
Extent(void *p, VG_View *vv, VG_Vector *a, VG_Vector *b)
{
	VG_Point *pt = p;
	VG_Vector pos = VG_Pos(pt);

	*a = pos;
	*b = pos;
}

static float
PointProximity(void *p, VG_View *vv, VG_Vector *vPt)
{
	VG_Point *pt = p;
	VG_Vector pos = VG_Pos(pt);
	float d;

	d = VG_Distance(pos, *vPt);
	*vPt = pos;
	return (d);
}

static void
Move(void *p, VG_Vector vPos, VG_Vector vRel)
{
	VG_SetPosition(p, vPos);
}

static void *
Edit(void *p, VG_View *vv)
{
	VG_Point *vp = p;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_NumericalNewFlt(box, 0, NULL, _("Render size: "), &vp->size);
	return (box);
}

VG_NodeOps vgPointOps = {
	N_("Point"),
	&vgIconPoints,
	sizeof(VG_Point),
	Init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	Draw,
	Extent,
	PointProximity,
	NULL,			/* lineProximity */
	NULL,			/* deleteNode */
	Move,
	Edit
};
