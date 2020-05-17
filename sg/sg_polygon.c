/*
 * Copyright (c) 2011-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Construction geometry: Polygon.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
		
#define SG_POLYGON_PRECISION 0.2	/* For plane intersection test */

SG_Polygon *
SG_PolygonNew(void *parent, const char *name, const M_Polygon *mp)
{
	SG_Polygon *poly;

	poly = Malloc(sizeof(SG_Polygon));
	AG_ObjectInit(poly, &sgPolygonClass);
	if (name) {
		AG_ObjectSetNameS(poly, name);
	} else {
		OBJECT(poly)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, poly);
	if (mp != NULL) {
		M_PolygonCopy(&poly->P, mp);
	}
	return (poly);
}

static void
Init(void *_Nonnull obj)
{
	SG_Polygon *poly = obj;

	M_PolygonInit(&poly->P);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Polygon *poly = obj;

	poly->P = M_PolygonRead(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Polygon *poly = obj;

	M_PolygonWrite(ds, &poly->P);
	return (0);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Polygon *poly = obj;
	SG_Geom *geom = SGGEOM(poly);
	Uint i;

	SG_GeomDrawBegin(geom);

	GL_Begin(GL_LINE_LOOP);
	if (SGNODE(geom)->flags & SG_NODE_SELECTED) {
		GL_Color3ub(0, 255, 0);
	} else {
		GL_Color4v(&geom->c);
	}
	for (i = 0; i < poly->P.n; i++) {
		GL_Vertex2v(&poly->P.v[i]);
	}
	GL_End();

	SG_GeomDrawEnd(geom);
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	SG_Polygon *poly = obj;
	M_Geom3 xg;

	switch (g.type) {
	case M_LINE:
		{
			M_Vector3 p1, p2, u;
			M_Real d, n, Si;
			M_Vector3 xp;
			
			M_LineToPts3(g.g.line, &p1, &p2);
			u = M_VecSub3(p2, p1);
			d = M_VecDot3(M_VecK3(), u);
			n = -M_VecDot3(M_VecK3(), p1);

			if (Fabs(d) < SG_POLYGON_PRECISION) {
				if (n < SG_POLYGON_PRECISION) {
					if (S != NULL) {
						xg.type = M_LINE;
						xg.g.line = g.g.line;
						M_GeomSetAdd3(S, &xg);
					}
					return (1);
				} else {
					return (0);
				}
			}
			Si = n/d;
			if (Si < 0.0 || Si > 1.0)
				return (0);

			xp = M_VecAdd3(g.g.line.p, M_VecScale3p(&u,Si));
			if (M_PointInPolygon(&poly->P, M_VECTOR2(xp.x,xp.y))) {
				if (S != NULL) {
					xg.type = M_POINT;
					xg.g.point = xp;
					M_GeomSetAdd3(S, &xg);
				}
				return (1);
			} else {
				return (0);
			}
		}
		break;
	default:
		return (-1);
	}
	return (0);
}

SG_NodeClass sgPolygonClass = {
	{
		"SG_Node:SG_Geom:SG_Polygon",
		sizeof(SG_Polygon),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		Load,
		Save,
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	Draw,
	Intersect,
	NULL			/* edit */
};
