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
 * Circle element.
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
	VG_Circle *vc = p;

	vc->p = NULL;
	vc->r = 0.025f;
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
	VG_Circle *vc = p;

	if ((vc->p = VG_ReadRef(ds, vc, "Point")) == NULL) {
		return (-1);
	}
	vc->r = AG_ReadFloat(ds);
	return (0);
}

static void
Save(void *p, AG_DataSource *ds)
{
	VG_Circle *vc= p;

	VG_WriteRef(ds, vc->p);
	AG_WriteFloat(ds, vc->r);
}

static void
Draw(void *p, VG_View *vv)
{
	VG_Circle *vc = p;
	VG_Vector vCenter = VG_Pos(vc->p);
	int x, y, r;

	VG_GetViewCoords(vv, vCenter, &x, &y);
	r = (int)(vc->r*vv->scale);
	AG_DrawCircle(vv, x, y, r, VG_MapColorRGB(VGNODE(vc)->color));
}

static void
Extent(void *p, VG_View *vv, VG_Vector *a, VG_Vector *b)
{
	VG_Circle *vc = p;
	VG_Vector vCenter = VG_Pos(vc->p);

	a->x = vCenter.x - vc->r;
	a->y = vCenter.y - vc->r;
	b->x = vCenter.x + vc->r;
	b->y = vCenter.y + vc->r;
}

static float
PointProximity(void *p, VG_View *vv, VG_Vector *vPt)
{
	VG_Circle *vc = p;
	VG_Vector vCenter = VG_Pos(vc->p);
	float theta = Atan2(vPt->y - vCenter.y,
	                    vPt->x - vCenter.x);
	VG_Vector vNear;
	float d;

	vNear.x = vCenter.x + vc->r*Cos(theta);
	vNear.y = vCenter.y + vc->r*Sin(theta);
	d = VG_Distance(*vPt, vNear);
	*vPt = vNear;
	return (d);
}

static void
Delete(void *p)
{
	VG_Circle *vc = p;

	if (VG_DelRef(vc, vc->p) == 0)
		VG_Delete(vc->p);
}

static void
Move(void *p, VG_Vector vCurs, VG_Vector vRel)
{
	VG_Circle *vc = p;

	vc->r = VG_Distance(VG_Pos(vc->p), vCurs);
}

static void *
Edit(void *p, VG_View *vv)
{
	VG_Circle *vc = p;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_NumericalNewFlt(box, 0, NULL, _("Radius: "), &vc->r);
	return (box);
}

VG_NodeOps vgCircleOps = {
	N_("Circle"),
	&vgIconCircle,
	sizeof(VG_Circle),
	Init,
	NULL,			/* destroy */
	Load,
	Save,
	Draw,
	Extent,
	PointProximity,
	NULL,			/* lineProximity */
	Delete,
	Move,
	Edit
};
