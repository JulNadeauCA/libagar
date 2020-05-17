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
 * Construction geometry: Triangle.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Triangle *
SG_TriangleNew(void *parent, const char *name, const M_Triangle3 *mt)
{
	SG_Triangle *tri;

	tri = Malloc(sizeof(SG_Triangle));
	AG_ObjectInit(tri, &sgTriangleClass);
	if (name) {
		AG_ObjectSetNameS(tri, name);
	} else {
		OBJECT(tri)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, tri);
#if 0
	if (mt != NULL) {
		tri->T = *mt;
	}
#endif
	return (tri);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Triangle *tri = obj;
	SG_Geom *geom = SGGEOM(tri);

	SG_GeomDrawBegin(geom);

	GL_Begin(GL_LINE_LOOP);
	GL_Color4v(&geom->c);
	GL_Vertex3(-0.5, +0.5, 0.0);
	GL_Vertex3( 0.0, -0.5, 0.0);
	GL_Vertex3(+0.5, +0.5, 0.0);
	GL_End();

	SG_GeomDrawEnd(geom);
}

SG_NodeClass sgTriangleClass = {
	{
		"SG_Node:SG_Geom:SG_Triangle",
		sizeof(SG_Triangle),
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
	NULL,			/* intersect */
	NULL			/* edit */
};
