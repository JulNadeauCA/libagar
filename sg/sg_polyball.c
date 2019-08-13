/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Ideal sphere, approximated as a SG_Object polyhedron.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#define X 0.525731112119133606
#define Z 0.850650808352039932

static const M_Real IsoVtx[12][3] = {
	{-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
	{0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
	{Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};

static const Uint IsoIdx[20][3] = {
	{1,4,0}, {4,9,0}, {4,5,9}, {8,5,4}, {1,8,4},
	{1,10,8}, {10,3,8}, {8,3,5}, {3,2,5}, {3,7,2},
	{3,10,7}, {10,6,7}, {6,11,7}, {6,0,11}, {6,1,0},
	{10,1,6}, {11,0,9}, {2,11,9}, {5,2,9}, {11,2,7}
};

SG_Polyball *
SG_PolyballNew(void *parent, const char *name, const M_Sphere *ms)
{
	SG_Polyball *ball;

	ball = Malloc(sizeof(SG_Polyball));
	AG_ObjectInit(ball, &sgPolyballClass);
	if (name) {
		AG_ObjectSetNameS(ball, name);
	} else {
		OBJECT(ball)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, ball);
	if (ms != NULL) {
		SG_Translatev(ball, ms->p);
		SG_Scale(ball, ms->r);
	}
	return (ball);
}

static void
Init(void *_Nonnull obj)
{
	SG_Polyball *ball = obj;

	ball->flags = 0;
	SG_PolyballSetSubdiv(ball, 1);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_Polyball *ball = obj;
	int s;

	s = (int)AG_ReadUint16(buf);
	SG_PolyballSetSubdiv(ball, s);
	return (0);
}

static int
Save(void *_Nonnull p, AG_DataSource *_Nonnull buf)
{
	SG_Polyball *ball = p;

	AG_WriteUint16(buf, (Uint16)ball->subdiv);
	return (0);
}

static void
Subdivide(SG_Polyball *_Nonnull ball, const M_Vector3 *_Nonnull v1,
    const M_Vector3 *_Nonnull v2, const M_Vector3 *_Nonnull v3, int depth)
{
	M_Vector3 v12, v23, v31;
	static double st = 0.0;

	st += 0.01;
	if (depth == 0) {
		int vtx0, vtx1, vtx2;

		/*
		 * Since this is a unit sphere, the vertices and vectors
		 * have the same value.
		 */
		vtx0 = SG_VertexNewvn(ball, v1, v1);
		vtx1 = SG_VertexNewvn(ball, v2, v2);
		vtx2 = SG_VertexNewvn(ball, v3, v3);
		SG_FacetFromTri3(ball, vtx0,vtx1,vtx2);
		return;
	}
	v12 = M_VecAvg3p(v1, v2);
	v23 = M_VecAvg3p(v2, v3);
	v31 = M_VecAvg3p(v3, v1);
	M_VecNorm3v(&v12);
	M_VecNorm3v(&v23);
	M_VecNorm3v(&v31);
	Subdivide(ball, v1, &v12, &v31, depth-1);
	Subdivide(ball, v2, &v23, &v12, depth-1);
	Subdivide(ball, v3, &v31, &v23, depth-1);
	Subdivide(ball, &v12, &v23, &v31, depth-1);
}

/* Change the subdivision level. */
void
SG_PolyballSetSubdiv(SG_Polyball *ball, int subdiv)
{
	int i;

	AG_ObjectLock(ball);

	ball->subdiv = subdiv;
	SG_ObjectFreeGeometry(ball);

	/* Every subdivision level multiplies the edge count fourfold. */
	SG_EdgeRehash(ball,
	    (Uint)(120.0*Pow(4.0, (M_Real)ball->subdiv - 1.0)));
	SG_FacetRehash(ball, SGOBJECT(ball)->nEdgeTbl/1.5);

	for (i = 0; i < 20; i++) {
		M_Vector3 v1 = M_RealvToVector3(IsoVtx[IsoIdx[i][0]]);
		M_Vector3 v2 = M_RealvToVector3(IsoVtx[IsoIdx[i][1]]);
		M_Vector3 v3 = M_RealvToVector3(IsoVtx[IsoIdx[i][2]]);

		Subdivide(ball, &v1, &v2, &v3, ball->subdiv);
	}
	
	AG_ObjectUnlock(ball);
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	M_Real r = 1;
	M_Geom3 xg;

	switch (g.type) {
	case M_POINT:
		if (M_VecLen3(g.g.point) <= 1.0) {
			if (S != NULL) {
				xg.type = M_POINT;
				xg.g.point = g.g.point;
				M_GeomSetAdd3(S, &xg);
			}
			return (1);
		}
		break;
	case M_LINE:
		{
			M_Real a, b, c, bb4ac;
			M_Vector3 p = g.g.line.p;
			M_Vector3 dp = g.g.line.d;

			a =    dp.x*dp.x + dp.y*dp.y + dp.z*dp.z;
			b = 2*(dp.x* p.x + dp.y* p.y + dp.z* p.z);
			c =    (p.x *p.x +  p.y* p.y +  p.z* p.z) - r*r;
			bb4ac = b*b - 4*a*c;
			if (Fabs(a) < M_MACHEP || bb4ac < 0)
				return (0);

			if (S != NULL) {
				M_Real mu1, mu2;

				mu1 = (-b + M_Sqrt(bb4ac)) / (2*a);
				mu2 = (-b - M_Sqrt(bb4ac)) / (2*a);

				/*
				 * Points of intersection at:
				 * p = p1 + mu1 (p2 - p1)
				 * p = p1 + mu2 (p2 - p1)
				 */
				xg.type = M_LINE;
				xg.g.line = M_LineFromPts3(
				    M_VecAdd3(p, M_VecScale3(dp, mu1)),
				    M_VecAdd3(p, M_VecScale3(dp, mu2)));
				M_GeomSetAdd3(S, &xg);
			}
			return (1);
		}
		break;
	default:
		return (-1);
	}
	return (0);
}

static void
UpdateSubdiv(AG_Event *_Nonnull event)
{
	SG_Polyball *ball = AG_PTR(1);
	SG_View *sgv = AG_PTR(2);

	SG_PolyballSetSubdiv(ball, ball->subdiv);
	AG_Redraw(sgv);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Polyball *ball = obj;
	AG_Box *vBox, *hBox;
	AG_Numerical *num;

	vBox = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	hBox = AG_BoxNewHoriz(vBox, AG_BOX_HFILL);
	num = AG_NumericalNewS(hBox, AG_NUMERICAL_HFILL|AG_NUMERICAL_EXCL, NULL, _("Subdivisions: "));
	AG_BindInt(num, "value", &ball->subdiv);
	AG_SetInt(num, "min", 1);
	AG_SetInt(num, "max", 5);
	AG_SetEvent(num, "numerical-changed", UpdateSubdiv, "%p,%p", ball, sgv);

	return (vBox);
}

SG_NodeClass sgPolyballClass = {
	{
		"SG_Node:SG_Object:SG_Polyball",
		sizeof(SG_Polyball),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		Load,
		Save,
		SG_NodeEdit
	},
	SG_ObjectMenuInstance,
	NULL,			/* menuClass */
	NULL,			/* draw */
	Intersect,
	Edit
};
