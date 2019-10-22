/*
 * Copyright (c) 2005-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Polygon entity.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/iconmgr.h>
#include <agar/gui/opengl.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

VG_Polygon *
VG_PolygonNew(void *pNode)
{
	VG_Polygon *ply = Malloc(sizeof(VG_Polygon));

	VG_NodeInit(ply, &vgPolygonOps);
	VG_NodeAttach(pNode, ply);
	return (ply);
}

void
VG_PolygonSetOutline(VG_Polygon *ply, int flag)
{
	ply->outline = flag;
}

Uint
VG_PolygonVertex(VG_Polygon *ply, VG_Point *pt)
{
	VG *vg = VGNODE(ply)->vg;

	AG_ObjectLock(vg);

	ply->pts = Realloc(ply->pts, (ply->nPts + 1)*sizeof(VG_Point *));
	ply->pts[ply->nPts] = pt;
	VG_AddRef(ply, pt);

	AG_ObjectUnlock(vg);
	return (ply->nPts++);
}

void
VG_PolygonDelVertex(VG_Polygon *ply, Uint vtx)
{
	VG *vg = VGNODE(ply)->vg;

	AG_ObjectLock(vg);
	if (vtx < ply->nPts) {
		VG_DelRef(ply, ply->pts[vtx]);
		if (vtx < ply->nPts-1) {
			memmove(&ply->pts[vtx], &ply->pts[vtx+1],
			    (ply->nPts - vtx - 1)*sizeof(VG_Point *));
		}
		ply->nPts--;
	}
	AG_ObjectUnlock(vg);
}

static void
Init(void *_Nonnull obj)
{
	VG_Polygon *vp = obj;

	vp->outline = 0;
	vp->pts = NULL;
	vp->nPts = 0;
	vp->ints = NULL;
	vp->nInts = 0;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	VG_Polygon *vp = obj;
	Uint i;

	vp->outline = (int)AG_ReadUint8(ds);
	vp->nPts = (Uint)AG_ReadUint8(ds);
	vp->pts = Malloc(vp->nPts*sizeof(VG_Point *));
	for (i = 0; i < vp->nPts; i++) {
		if ((vp->pts[i] = VG_ReadRef(ds, vp, "Point")) == NULL)
			return (-1);
	}
	return (0);
}

static void
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	VG_Polygon *vp = obj;
	Uint i;

	AG_WriteUint8(ds, (Uint8)vp->outline);
	AG_WriteUint8(ds, (Uint8)vp->nPts);
	for (i = 0; i < vp->nPts; i++)
		VG_WriteRef(ds, vp->pts[i]);
}

static void
Destroy(void *_Nonnull obj)
{
	VG_Polygon *vp = obj;

	Free(vp->pts);
	Free(vp->ints);
}

static int
CompareInts(const void *_Nonnull p1, const void *_Nonnull p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}

static void
DrawOutline(VG_Polygon *_Nonnull vp, VG_View *_Nonnull vv)
{
	AG_Color c = VG_MapColorRGB(VGNODE(vp)->color);
	int Ax, Ay, Bx, By, Cx, Cy;
	int i;

	if (vp->nPts < 2) {
		return;
	}
	VG_GetViewCoords(vv, VG_Pos(vp->pts[0]), &Ax,&Ay);
	Cx = Ax;
	Cy = Ay;
	for (i = 1; i < vp->nPts; i++) {
		VG_GetViewCoords(vv, VG_Pos(vp->pts[i]), &Bx,&By);
		AG_DrawLine(vv, Ax,Ay, Bx,By, &c);
		Ax = Bx;
		Ay = By;
	}
	AG_DrawLine(vv, Cx,Cy, Ax,Ay, &c);
}

static void
DrawFB(VG_Polygon *_Nonnull vp, VG_View *_Nonnull vv)
{
	AG_Color c = VG_MapColorRGB(VGNODE(vp)->color);
	int y, x1, y1, x2, y2;
	int ign, miny, maxy;
	int i, i1, i2;
	Uint nPts = vp->nPts;
	int nInts;

	if (vp->ints == NULL) {
		vp->ints = Malloc(nPts*sizeof(int));
		vp->nInts = nPts;
	} else {
		if (nPts > vp->nInts) {
			vp->ints = Realloc(vp->ints, nPts*sizeof(int));
			vp->nInts = nPts;
		}
	}

	/* Find Y maxima */
	VG_GetViewCoords(vv, VG_Pos(vp->pts[0]), &ign, &miny);
	maxy = miny;
	for (i = 1; i < nPts; i++) {
		int vy;
	
		VG_GetViewCoords(vv, VG_Pos(vp->pts[i]), &ign, &vy);
		if (vy < miny) {
			miny = vy;
		} else if (vy > maxy) {
			maxy = vy;
		}
	}

	/* Find the intersections. */
	for (y = miny; y <= maxy; y++) {
		nInts = 0;
		for (i = 0; i < nPts; i++) {
			if (i == 0) {
				i1 = nPts - 1;
				i2 = 0;
			} else {
				i1 = i - 1;
				i2 = i;
			}
			VG_GetViewCoords(vv, VG_Pos(vp->pts[i1]), &ign, &y1);
			VG_GetViewCoords(vv, VG_Pos(vp->pts[i2]), &ign, &y2);
			if (y1 < y2) {
				VG_GetViewCoords(vv, VG_Pos(vp->pts[i1]),
				    &x1, &ign);
				VG_GetViewCoords(vv, VG_Pos(vp->pts[i2]),
				    &x2, &ign);
			} else if (y1 > y2) {
				VG_GetViewCoords(vv, VG_Pos(vp->pts[i1]),
				    &x2, &y2);
				VG_GetViewCoords(vv, VG_Pos(vp->pts[i2]),
				    &x1, &y1);
			} else {
				continue;
			}
			if (((y >= y1) && (y < y2)) ||
			    ((y == maxy) && (y > y1) && (y <= y2))) {
				vp->ints[nInts++] =
				    (((y - y1) << 16) / (y2 - y1)) *
				     (x2 - x1) + (x1 << 16);
			} 
		}

		/* TODO use a less general integer sort */
		qsort(vp->ints, nInts, sizeof(int), CompareInts);

		for (i = 0; i < nInts; i += 2) {
			int xa, xb;

			xa = vp->ints[i] + 1;
			xa = (xa >> 16) + ((xa & 0x8000) >> 15);
			xb = vp->ints[i+1] - 1;
			xb = (xb >> 16) + ((xb & 0x8000) >> 15);
			AG_DrawLineH(vv, xa, xb, y, &c);
		}
	}
}

static void
Draw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Polygon *vp = obj;

	if (vp->nPts < 3 || vp->outline) {
		DrawOutline(vp, vv);
		return;
	}
#ifdef HAVE_OPENGL
	if (AGDRIVER_CLASS(WIDGET(vv)->drv)->flags & AG_DRIVER_OPENGL) {
		VG_Color *c = &VGNODE(vp)->color;
		int x1 = WIDGET(vv)->rView.x1;
		int y1 = WIDGET(vv)->rView.y1;
		Uint i;

		glBegin(GL_POLYGON);
		glColor3ub(c->r, c->g, c->b);
		for (i = 0; i < vp->nPts; i++) {
			int x,y;
			VG_GetViewCoords(vv, VG_Pos(vp->pts[i]), &x,&y);
			glVertex2i(x1+x, y1+y);
		}
		glEnd();
	} else
#endif /* HAVE_OPENGL */
	{
		DrawFB(vp, vv);
	}
}

static void
Extent(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull a,
    VG_Vector *_Nonnull b)
{
	VG_Polygon *vp = obj;
	VG_Vector v;
	int i;

	if (vp->nPts < 1) {
		a->x = b->x = 0;
		a->y = b->y = 0;
		return;
	}
	v = VG_Pos(vp->pts[0]);
	a->x = b->x = v.x;
	a->y = b->y = v.y;
	for (i = 0; i < vp->nPts; i++) {
		v = VG_Pos(vp->pts[i]);
		if (v.x < a->x) { a->x = v.x; }
		if (v.y < a->y) { a->y = v.y; }
		if (v.x > b->x) { b->x = v.x; }
		if (v.y > b->y) { b->y = v.y; }
	}
}

static float
PointProximity(void *_Nonnull obj, VG_View *_Nonnull vv, VG_Vector *_Nonnull vPt)
{
	VG_Polygon *vp = obj;
	float d, dMin;
	VG_Vector vInt, A, B, C, m;
	int i;

	if (vp->nPts < 1)
		return (AG_FLT_MAX);

	dMin = AG_FLT_MAX;
	m.x = 0.0f;
	m.y = 0.0f;
	C = A = VG_Pos(vp->pts[0]);
	for (i = 1; i < vp->nPts; i++) {
		B = VG_Pos(vp->pts[i]);
		vInt = *vPt;
		d = VG_PointLineDistance(A, B, &vInt);
		if (d < dMin) {
			dMin = d;
			m = vInt;
		}
		A = B;
	}
	vInt = *vPt;
	d = VG_PointLineDistance(C, A, &vInt);
	if (d < dMin) {
		dMin = d;
		m = vInt;
	}
	if (dMin < AG_FLT_MAX) {
		vPt->x = m.x;
		vPt->y = m.y;
	}
	return (dMin);
}

static void
Delete(void *_Nonnull obj)
{
	VG_Polygon *vp = obj;
	Uint i;

	for (i = 0; i < vp->nPts; i++) {
		if (VG_DelRef(vp, vp->pts[i]) == 0)
			VG_Delete(vp->pts[i]);
	}
}

static void *_Nonnull
Edit(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Polygon *vp = obj;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

#ifdef AG_ENABLE_STRING
	AG_LabelNewPolled(box, AG_LABEL_HFILL, _("Points: %d"), &vp->nPts);
#endif
	AG_SeparatorNewHoriz(box);
	AG_CheckboxNewInt(box, 0, _("Render outline"), &vp->outline);
	return (box);
}

VG_NodeOps vgPolygonOps = {
	N_("Polygon"),
	&vgIconPolygon,
	sizeof(VG_Polygon),
	Init,
	Destroy,
	Load,
	Save,
	Draw,
	Extent,
	PointProximity,
	NULL,			/* lineProximity */
	Delete,
	NULL,			/* moveNode */
	Edit
};
