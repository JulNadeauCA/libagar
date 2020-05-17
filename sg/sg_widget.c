/*
 * Copyright (c) 2012-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Widget control.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Widget *
SG_WidgetNew(void *parent, enum sg_widget_style style, const char *name)
{
	SG_Widget *w;

	w = Malloc(sizeof(SG_Widget));
	AG_ObjectInit(w, &sgWidgetClass);
	if (name) {
		AG_ObjectSetNameS(w, name);
	} else {
		OBJECT(w)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, w);
	w->style = style;
	return (w);
}

static void
Init(void *_Nonnull obj)
{
	SG_Widget *w = obj;

	w->style = SG_WIDGET_DISC;
	w->flags = 0;
	w->size = 0.10;
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Widget *w = obj;
	SG_Geom *geom = SGGEOM(w);
	float modelview[16];
	int i, j;
	float z;

	GL_PushAttrib(GL_LIGHTING_BIT|GL_LINE_BIT);
	GL_Disable(GL_LIGHTING);
	GL_LineWidth(geom->wd);
	GL_PushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (i == j) {
				modelview[i*4 + j] = 1.0;
			} else {
				modelview[i*4 + j] = 0.0;
			}
		}
	}
	glLoadMatrixf(modelview);

	if (SGNODE(w)->flags & SG_NODE_SELECTED) {
		geom->c = M_ColorRGB(255, 255, 128);
	} else {
		geom->c = M_ColorRGB(255, 255, 0);
	}

	switch (w->style) {
	case SG_WIDGET_DISC:
		for (z = -0.1f; z <= 0.1f; z += 0.033f) {
			GL_Begin(GL_POLYGON);
			GL_Color3ub(128, 128, 128);
			for (i = 0; i < 7; i++) {
				GL_Vertex3(w->size*Cos((2.0*M_PI*i)/8),
				           w->size*Sin((2.0*M_PI*i)/8),
					   z);
			}
			GL_End();
		}
		GL_Begin(GL_POLYGON);
		GL_Color3ub(200, 200, 200);
		for (i = 0; i < 7; i++) {
			GL_Vertex3((w->size/2)*Cos((2.0*M_PI*i)/8),
			           (w->size/2)*Sin((2.0*M_PI*i)/8),
				   -0.15);
		}
		GL_End();
		GL_Begin(GL_POLYGON);
		GL_Color3ub(200, 200, 200);
		for (i = 0; i < 7; i++) {
			GL_Vertex3((w->size/2)*Cos((2.0*M_PI*i)/8),
			           (w->size/2)*Sin((2.0*M_PI*i)/8),
				   +0.15);
		}
		GL_End();
		break;
	case SG_WIDGET_SQUARE:
		GL_Begin(GL_LINE_LOOP);
		if (SGNODE(geom)->flags & SG_NODE_SELECTED) {
			GL_Color3ub(0, 255, 0);
		} else {
			GL_Color4v(&geom->c);
		}
		GL_Vertex2(0.0, 0.0);
		GL_Vertex2(w->size, 0.0);
		GL_Vertex2(w->size, w->size);
		GL_Vertex2(0.0, w->size);
		GL_End();
		break;
	}

	GL_PopMatrix();
	GL_PopAttrib();
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *S)
{
	SG_Widget *w = obj;
	M_Geom3 xg;
	M_Real r = w->size/2.0;
	M_Real a, b, c, bb4ac;
	M_Vector3 p = g.g.line.p;
	M_Vector3 dp = g.g.line.d;

	if (g.type != M_LINE)
		return (-1);

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
	return (0);
}

SG_NodeClass sgWidgetClass = {
	{
		"SG_Node:SG_Geom:SG_Widget",
		sizeof(SG_Widget),
		{ 0,0 },
		Init,
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
