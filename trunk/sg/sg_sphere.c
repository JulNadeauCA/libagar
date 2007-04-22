/*
 * Copyright (c) 2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/core/core.h>

#include "sg.h"
#include "sg_gui.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#define X 0.525731112119133606
#define Z 0.850650808352039932

static const SG_Vector IsoVtx[12] = {
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

SG_Sphere *
SG_SphereNew(void *pnode, const char *name)
{
	SG_Sphere *sph;

	sph = Malloc(sizeof(SG_Sphere), M_SG);
	SG_SphereInit(sph, name);
	SG_NodeAttach(pnode, sph);
	return (sph);
}

void
SG_SphereInit(void *p, const char *name)
{
	SG_Sphere *sph = p;

	SG_ObjectInit(sph, name);
	SGNODE(sph)->ops = &sgSphereOps;
	sph->radius = 1.0;
	sph->tesslvl = 1;
	SG_SphereGen(sph);
}

int
SG_SphereLoad(void *p, AG_Netbuf *buf)
{
	SG_Sphere *sph = p;

	if (SG_ObjectLoad(sph, buf) == -1) {
		return (-1);
	}
	sph->radius = SG_ReadReal(buf);
	sph->tesslvl = AG_ReadUint16(buf);

	if (sph->radius <= 0.0) {
		AG_SetError("Sphere has <=0 radius");
		return (-1);
	}
	return (0);
}

int
SG_SphereSave(void *p, AG_Netbuf *buf)
{
	SG_Sphere *sph = p;

	if (SG_ObjectSave(sph, buf) == -1) {
		return (-1);
	}
	SG_WriteReal(buf, sph->radius);
	AG_WriteUint16(buf, sph->tesslvl);
	return (0);
}

static void
Generate(AG_Event *event)
{
	SG_SphereGen(AG_PTR(1));
}

void
SG_SphereEdit(void *p, AG_Widget *box, SG_View *sgv)
{
	SG_Sphere *sph = p;
	AG_Button *btn;

	SG_ObjectEdit(p, box, sgv);

	SG_SpinReal(box, _("Radius"), &sph->radius);
	SG_SpinInt(box, _("Subdivisions"), &sph->tesslvl);
	btn = AG_ButtonAct(box, AG_BUTTON_HFILL, _("Generate"),
	    Generate, "%p", sph);
}

static void
Subdivide(SG_Sphere *sph, const SG_Vector *v1, const SG_Vector *v2,
    const SG_Vector *v3, int depth)
{
	SG_Vector v12, v23, v31;
	int i;

	if (depth == 0) {
		/*
		 * Since this is a unit sphere, the vertices and vectors
		 * have the same value.
		 */
		SG_FacetFromTri3(sph,
		    SG_VertexNewvn(sph, v1, v1),
		    SG_VertexNewvn(sph, v2, v2),
		    SG_VertexNewvn(sph, v3, v3));
		return;
	}
	SG_VectorAvg2v(&v12, v1, v2);
	SG_VectorAvg2v(&v23, v2, v3);
	SG_VectorAvg2v(&v31, v3, v1);
	SG_VectorNormv(&v12);
	SG_VectorNormv(&v23);
	SG_VectorNormv(&v31);
	Subdivide(sph, v1, &v12, &v31, depth-1);
	Subdivide(sph, v2, &v23, &v12, depth-1);
	Subdivide(sph, v3, &v31, &v23, depth-1);
	Subdivide(sph, &v12, &v23, &v31, depth-1);
}

void
SG_SphereGen(SG_Sphere *sph)
{
	int i;

	SG_ObjectFreeGeometry(sph);

	/* Every subdivision level multiplies the edge count by 4. */
	SG_EdgeRehash(sph, sph->tesslvl>0 ?
	    (Uint)(30.0*SG_Pow(4.0, (SG_Real)sph->tesslvl-1.0)) :
	    30);

	for (i = 0; i < 20; i++) {
		Subdivide(sph,
		    &IsoVtx[IsoIdx[i][0]],
		    &IsoVtx[IsoIdx[i][1]],
		    &IsoVtx[IsoIdx[i][2]], sph->tesslvl);
	}
}

void
SG_SphereDraw(void *p, SG_View *view)
{
	SG_Sphere *sph = p;
	SG_Real d;
	SG_Matrix T;
	int lod;
	
	d = SG_VectorDistance(SG_NodePos(view->cam),
	                      SG_NodePos(sph));

	if (d < 2.0) { lod = 4; }
	else if (d < 5.0) { lod = 3; } 
	else if (d < 20.0) { lod = 2; } 
	else if (d < 150.0) { lod = 1; } 
	else { lod = 0; }

	if (lod != sph->tesslvl) {
		sph->tesslvl = lod;
		SG_SphereGen(sph);
		dprintf("%s: dist=%f, lod=%d\n", SGNODE(sph)->name, d, lod);
	}
	SG_ObjectDraw(sph, view);
}

SG_NodeOps sgSphereOps = {
	"Object:Sphere",
	sizeof(SG_Sphere),
	0,
	SG_SphereInit,
	NULL,			/* destroy */
	SG_SphereLoad,
	SG_SphereSave,
	SG_SphereEdit,
	SG_ObjectMenuInstance,
	NULL,			/* menuClass */
	SG_SphereDraw
};
