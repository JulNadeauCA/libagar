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
 * Circular arc element.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/numerical.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

VG_Arc *
VG_ArcNew(void *pNode, VG_Point *pCenter, float r, float a1, float a2)
{
	VG_Arc *va = Malloc(sizeof(VG_Arc));

	VG_NodeInit(va, &vgArcOps);
	va->p = pCenter;
	va->r = r;
	va->a1 = a1;
	va->a2 = a2;
	VG_AddRef(va, pCenter);
	VG_NodeAttach(pNode, va);
	return (va);
}

void
VG_ArcCenter(VG_Arc *va, VG_Point *pCenter)
{
	VG *vg = VGNODE(va)->vg;

	AG_ObjectLock(vg);
	VG_DelRef(va, va->p);
	VG_AddRef(va, pCenter);
	va->p = pCenter;
	AG_ObjectUnlock(vg);
}

void
VG_ArcRadius(VG_Arc *va, double r)
{
	VG *vg = VGNODE(va)->vg;

	AG_ObjectLock(vg);
	va->r = r;
	AG_ObjectUnlock(vg);
}

static void
Init(void *p)
{
	VG_Arc *va = p;

	va->p = NULL;
	va->r = 0.5;
	va->a1 = 0.0f;
	va->a2 = 360.0f;
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
	VG_Arc *va = p;

	if ((va->p = VG_ReadRef(ds, va, "Point")) == NULL) {
		return (-1);
	}
	va->r = AG_ReadDouble(ds);
	va->a1 = AG_ReadFloat(ds);
	va->a2 = AG_ReadFloat(ds);
	return (0);
}

static void
Save(void *p, AG_DataSource *ds)
{
	VG_Arc *va = p;

	VG_WriteRef(ds, va->p);
	AG_WriteDouble(ds, va->r);
	AG_WriteFloat(ds, va->a1);
	AG_WriteFloat(ds, va->a2);
}

static void
Draw(void *p, VG_View *vv)
{
	VG_Arc *va = p;
	int a1 = (int)va->a1;
	int a2 = (int)va->a2;
	long r = (long)va->r;
	int x, y, xPos, yPos;
	int xPrev = 0, yPrev = 0;
	VG_Vector vCenter = VG_Pos(va->p);
	AG_Color c = VG_MapColorRGB(VGNODE(va)->color);
	int a;

	VG_GetViewCoords(vv, vCenter, &xPos, &yPos);

	while (a2 < a1) {
		a2 += 360;
	}
	for (a = a1; a <= a2; a++) {
		x = ((long)vg_cos_tbl[a % 360]*(long)r/1024) + xPos;
		y = ((long)vg_sin_tbl[a % 360]*(long)r/1024) + yPos;
		if (a != a1) {
			AG_DrawLine(vv, xPrev, yPrev, x, y, &c);
		}
		xPrev = x;
		yPrev = y;
	}
}

static void
Extent(void *p, VG_View *vv, VG_Vector *a, VG_Vector *b)
{
	VG_Arc *va = p;
	VG_Vector vCenter = VG_Pos(va->p);

	a->x = vCenter.x - va->r;
	a->y = vCenter.y - va->r;
	b->x = vCenter.x + va->r;
	b->y = vCenter.y + va->r;
}

static float
PointProximity(void *p, VG_View *vv, VG_Vector *vPt)
{
	VG_Arc *va = p;
	VG_Vector vCenter = VG_Pos(va->p);
	float a1 = VG_Radians(va->a1);
	float a2 = VG_Radians(va->a2);
	float d, theta;

	theta = Atan2(vPt->y - vCenter.y,
	              vPt->x - vCenter.x);
	if (theta < a1) {
		theta = a1;
	} else if (theta > a2) {
		theta = a2;
	}
	d = VG_Distance(vCenter, *vPt) - va->r;
	vPt->x = vCenter.x + va->r*Cos(theta);
	vPt->y = vCenter.y + va->r*Sin(theta);
	return (d);
}

static void
Delete(void *p)
{
	VG_Arc *va = p;

	if (VG_DelRef(va, va->p) == 0)
		VG_Delete(va->p);
}

static void
Move(void *p, VG_Vector vCurs, VG_Vector vRel)
{
	VG_Arc *va = p;

	va->r = VG_Distance(VG_Pos(va->p), vCurs);
}

static void *
Edit(void *p, VG_View *vv)
{
	VG_Arc *va = p;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_NumericalNewDbl(box, 0, NULL, _("Radius: "), &va->r);
	AG_NumericalNewFlt(box, 0, NULL, _("Start angle: "), &va->a1);
	AG_NumericalNewFlt(box, 0, NULL, _("End angle: "), &va->a2);

	return (box);
}

VG_NodeOps vgArcOps = {
	N_("Arc"),
	&vgIconBezier,
	sizeof(VG_Arc),
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
