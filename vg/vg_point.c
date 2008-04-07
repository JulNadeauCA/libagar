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

#include <core/limits.h>
#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"
#include "icons.h"

static void
Init(void *p)
{
	VG_Point *pt = p;

	pt->x = 0.0f;
	pt->y = 0.0f;
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
	VG_Point *vp = p;

	vp->x = AG_ReadFloat(ds);
	vp->y = AG_ReadFloat(ds);
	return (0);
}

static void
Save(void *p, AG_DataSource *ds)
{
	VG_Point *vp = p;

	AG_WriteFloat(ds, vp->x);
	AG_WriteFloat(ds, vp->y);
}

static void
Draw(void *p, VG_View *vv)
{
	VG_Point *pt = p;
	Uint32 c32 = VG_MapColorRGB(VGNODE(pt)->color);
	int x, y;

	VG_GetViewCoords(vv, VG_PointPos(pt), &x, &y);
	AG_DrawCircle(vv, x, y, 2, c32);
}

static void
Extent(void *p, VG_View *vv, VG_Rect *r)
{
	VG_Point *pt = p;

	r->x = pt->x;
	r->y = pt->y;
	r->w = 0;
	r->h = 0;
}

static float
PointProximity(void *p, VG_Vector *vPt)
{
	VG_Point *pt = p;
	float d;

	d = VG_Distance(VG_PointPos(pt), *vPt);
	vPt->x = pt->x;
	vPt->y = pt->y;
	return (d);
}

static void
Move(void *p, VG_Vector vCurs, VG_Vector vRel)
{
	VG_Point *pt = p;

	pt->x = vCurs.x;
	pt->y = vCurs.y;
}

const VG_NodeOps vgPointOps = {
	N_("Point"),
	&vgIconPoints,
	sizeof(VG_Point),
	Init,
	NULL,			/* destroy */
	Load,
	Save,
	Draw,
	Extent,
	PointProximity,
	NULL,			/* lineProximity */
	NULL,			/* deleteNode */
	Move
};
