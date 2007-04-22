/*
 * Copyright (c) 2005-2007 Hypertriton, Inc.
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

#include <GL/gl.h>
#include <GL/glu.h>

static GLenum sgGLprimitives[] = {
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_QUADS,
	GL_QUAD_STRIP,
	GL_POLYGON
};

SG_Vector3 sgMin, sgMax;

/* Render a wireframe box from 2 points. */
void
SG_WireBox2(SG_Vector p1, SG_Vector p2)
{
	glBegin(GL_LINE_LOOP);
	glVertex3d(p1.x, p1.y, p1.z);
	glVertex3d(p2.x, p1.y, p1.z);
	glVertex3d(p2.x, p2.y, p1.z);
	glVertex3d(p1.x, p2.y, p1.z);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3d(p2.x, p2.y, p2.z);
	glVertex3d(p1.x, p2.y, p2.z);
	glVertex3d(p1.x, p1.y, p2.z);
	glVertex3d(p2.x, p1.y, p2.z);
	glEnd();
	glBegin(GL_LINES);
	glVertex3d(p1.x, p1.y, p1.z);
	glVertex3d(p1.x, p1.y, p2.z);
	glVertex3d(p2.x, p1.y, p1.z);
	glVertex3d(p2.x, p1.y, p2.z);
	glVertex3d(p1.x, p2.y, p1.z);
	glVertex3d(p1.x, p2.y, p2.z);
	glVertex3d(p2.x, p2.y, p1.z);
	glVertex3d(p2.x, p2.y, p2.z);
	glEnd();
}

/* Build a tesselated rectangular surface given a plane and dimensions. */
void
SG_TessRect3(SG_Plane P, SG_Real w, SG_Real h, SG_Real s)
{
	SG_Real x, z;
	int flag = 0;
	SG_Real w2 = w/2;
	SG_Real h2 = h/2;

	glColor3ub(100, 100, 100);
	for (z = -w2; z < w2; z += s) {
		glShadeModel(GL_SMOOTH);
		SG_Begin(SG_TRIANGLE_STRIP);
		for (x = -h2; x < (h2 - s); x += s) {
			SG_Vertex3(x, fabs((-P.a*x - P.c*(z+s) - P.d)/P.b),
			    z+s);
			if (flag) {
				flag = 0;
			} else {
				flag = 1;
			}
			SG_Vertex3(x, fabs((-P.a*x - P.c*z - P.d)/P.b), z);
		}
		flag = !flag;
		SG_End();
	}
}

void
SG_SolidBox2(SG_Vector p1, SG_Vector p2)
{
	glBegin(GL_POLYGON);
	glVertex3d(p1.x, p1.y, p1.z);
	glVertex3d(p2.x, p1.y, p1.z);
	glVertex3d(p2.x, p2.y, p1.z);
	glVertex3d(p1.x, p2.y, p1.z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(p2.x, p2.y, p2.z);
	glVertex3d(p1.x, p2.y, p2.z);
	glVertex3d(p1.x, p1.y, p2.z);
	glVertex3d(p2.x, p1.y, p2.z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3d(p1.x, p1.y, p1.z);
	glVertex3d(p1.x, p1.y, p2.z);
	glVertex3d(p2.x, p1.y, p1.z);
	glVertex3d(p2.x, p1.y, p2.z);
	glVertex3d(p1.x, p2.y, p1.z);
	glVertex3d(p1.x, p2.y, p2.z);
	glVertex3d(p2.x, p2.y, p1.z);
	glVertex3d(p2.x, p2.y, p2.z);
	glEnd();
}
