/*
 * Copyright (c) 2004-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

VG_Circle *
VG_CircleNew(void *pNode, VG_Point *pCenter, float r)
{
	VG_Circle *vc = Malloc(sizeof(VG_Circle));

	VG_NodeInit(vc, &vgCircleOps);
	vc->p = pCenter;
	vc->r = r;
	VG_AddRef(vc, pCenter);
	VG_NodeAttach(pNode, vc);
	return (vc);
}

void
VG_CircleCenter(VG_Circle *vc, VG_Point *pCenter)
{
	VG *vg = VGNODE(vc)->vg;

	AG_ObjectLock(vg);
	VG_DelRef(vc, vc->p);
	VG_AddRef(vc, pCenter);
	vc->p = pCenter;
	AG_ObjectUnlock(vg);
}

static void
Init(void *_Nonnull obj)
{
	VG_Circle *vc = obj;

	vc->p = NULL;
	vc->r = 0.025;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	VG_Circle *vc = obj;

	if ((vc->p = VG_ReadRef(ds, vc, "Point")) == NULL) {
		return (-1);
	}
	vc->r = AG_ReadDouble(ds);
	return (0);
}

static void
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	VG_Circle *vc = obj;

	VG_WriteRef(ds, vc->p);
	AG_WriteDouble(ds, vc->r);
}

static void
Draw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Circle *vc = obj;
	VG_Vector vCenter = VG_Pos(vc->p);
	AG_Color c;
	int x, y, r;

	VG_GetViewCoords(vv, vCenter, &x, &y);
	r = (int)(vc->r*vv->scale);
	c = VG_MapColorRGB(VGNODE(vc)->color);
	AG_DrawCircle(vv, x, y, r, &c);
}

static void
Extent(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull a,
    VG_Vector *_Nonnull b)
{
	VG_Circle *vc = obj;
	VG_Vector vCenter = VG_Pos(vc->p);

	a->x = vCenter.x - vc->r;
	a->y = vCenter.y - vc->r;
	b->x = vCenter.x + vc->r;
	b->y = vCenter.y + vc->r;
}

static float
PointProximity(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull vPt)
{
	VG_Circle *vc = obj;
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
Delete(void *_Nonnull obj)
{
	VG_Circle *vc = obj;

	if (VG_DelRef(vc, vc->p) == 0)
		VG_Delete(vc->p);
}

static void
Move(void *_Nonnull obj, VG_Vector vCurs, VG_Vector vRel)
{
	VG_Circle *vc = obj;

	vc->r = VG_Distance(VG_Pos(vc->p), vCurs);
}

static void *_Nonnull
Edit(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Circle *vc = obj;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_NumericalNewDbl(box, 0, NULL, _("Radius: "), &vc->r);
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
