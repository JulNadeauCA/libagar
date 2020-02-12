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
 * Construction geometry: Sphere.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Sphere *
SG_SphereNew(void *parent, const char *name, const M_Sphere *si)
{
	SG_Sphere *sph;

	sph = Malloc(sizeof(SG_Sphere));
	AG_ObjectInit(sph, &sgSphereClass);
	if (name) {
		AG_ObjectSetNameS(sph, name);
	} else {
		OBJECT(sph)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, sph);
	if (si != NULL) {
		SG_Translatev(sph, si->p);
		sph->d = si->r*2.0;
	}
	return (sph);
}

static void
Init(void *_Nonnull obj)
{
	SG_Sphere *sph = obj;

	sph->d = 1.0;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Sphere *sph = obj;

	sph->d = M_ReadReal(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Sphere *sph = obj;

	M_WriteReal(ds, sph->d);
	return (0);
}

static void *_Nullable
Edit(void *_Nonnull p, SG_View *_Nullable sgv)
{
	SG_Sphere *sph = p;
	AG_Mutex *lock = &OBJECT(sph)->lock;
	AG_Box *box;
	AG_Numerical *num;

	box = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_HFILL);
	
	num = AG_NumericalNew(box, 0, NULL, _("Diameter"));
	M_BindRealMp(num, "value", &sph->d, lock);

	return (box);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Sphere *sph = obj;
	SG_Geom *geom = SGGEOM(sph);
	const int nEdges = 15; /* XXX */
	M_Real r = sph->d/2.0;
	float m[16];
	int i, j;

	SG_GeomDrawBegin(geom);

	/* Render the sphere outline. */
	GL_PushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (i == j) {
				m[i*4 + j] = 1.0;
			} else {
				m[i*4 + j] = 0.0;
			}
		}
	}
	glLoadMatrixf(m);
	
	GL_Begin(GL_LINE_LOOP);
	GL_Color4v(&geom->c);
	for (i = 0; i < nEdges; i++) {
		GL_Vertex2(r*Cos((2.0*M_PI*i)/nEdges),
		           r*Sin((2.0*M_PI*i)/nEdges));
	}
	GL_End();

	GL_PopMatrix();

	SG_GeomDrawEnd(geom);
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	SG_Sphere *sph = obj;
	M_Geom3 xg;

	switch (g.type) {
	case M_POINT:
		if (M_VecLen3(g.g.point) <= sph->d) {
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
			M_Real r = sph->d/2.0;
			M_Real a, b, c, bb4ac;
			M_Vector3 p = g.g.line.p;
			M_Vector3 dp = g.g.line.d;

			a = dp.x*dp.x + dp.y*dp.y + dp.z*dp.z;
			b = 2*(dp.x*p.x + dp.y*p.y + dp.z*p.z);
			c = (p.x*p.x + p.y*p.y + p.z*p.z) - r*r;
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

SG_NodeClass sgSphereClass = {
	{
		"SG_Node:SG_Geom:SG_Sphere",
		sizeof(SG_Sphere),
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
	Edit
};
