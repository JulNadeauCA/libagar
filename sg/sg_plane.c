/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Reference Geometry: Plane.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#define SG_PLANE_PRECISION 0.2

SG_Plane *
SG_PlaneNew(void *parent, const char *name)
{
	SG_Plane *po;

	po = Malloc(sizeof(SG_Plane));
	AG_ObjectInit(po, &sgPlaneClass);
	if (name) {
		AG_ObjectSetNameS(po, name);
	} else {
		OBJECT(po)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, po);
	return (po);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Geom *geom = obj;

	SG_GeomDrawBegin(geom);

	GL_Begin(GL_LINE_LOOP);
	if (SGNODE(geom)->flags & SG_NODE_SELECTED) {
		GL_Color3ub(0, 255, 0);
	} else {
		GL_Color4v(&geom->c);
	}
	GL_Vertex3(-0.5, +0.5, 0.0);
	GL_Vertex3(-0.5, -0.5, 0.0);
	GL_Vertex3(+0.5, -0.5, 0.0);
	GL_Vertex3(+0.5, +0.5, 0.0);
	GL_End();

	SG_GeomDrawEnd(geom);
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	M_Geom3 xg;

	switch (g.type) {
	case M_LINE:
		{
			M_Vector3 p1, p2, u;
			M_Real d, n, Si;

			M_LineToPts3(g.g.line, &p1, &p2);
			u = M_VecSub3(p2, p1);
			d = M_VecDot3(M_VecK3(), u);
			n = -M_VecDot3(M_VecK3(), p1);

			if (Fabs(d) < SG_PLANE_PRECISION) {
				if (n < SG_PLANE_PRECISION) {
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
			if (Si < 0 || Si > 1) {
				return (0);
			}
			if (S != NULL) {
				xg.type = M_POINT;
				xg.g.point = M_VecAdd3(p1, M_VecScale3p(&u,Si));
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

SG_NodeClass sgPlaneClass = {
	{
		"SG_Node:SG_Geom:SG_Plane",
		sizeof(SG_Plane),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	Draw,
	Intersect,
	NULL			/* edit */
};
