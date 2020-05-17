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
 * Construction geometry: Circle.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Circle *
SG_CircleNew(void *parent, const char *name, const M_Circle3 *mc)
{
	SG_Circle *c;

	c = Malloc(sizeof(SG_Circle));
	AG_ObjectInit(c, &sgCircleClass);
	if (name) {
		AG_ObjectSetNameS(c, name);
	} else {
		OBJECT(c)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, c);
	if (mc != NULL) {
		SG_Translatev(c, mc->p);
		SG_Scale(c, mc->r);
	}
	return (c);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Circle *c = obj;
	SG_Geom *geom = SGGEOM(c);
	const int nEdges = 10; /* XXX */
	int i;

	SG_GeomDrawBegin(geom);
	
	GL_Begin(GL_LINE_LOOP);
	GL_Color4v(&geom->c);
	for (i = 0; i < nEdges; i++) {
		GL_Vertex2(Cos((2.0*M_PI*i)/nEdges),
		           Sin((2.0*M_PI*i)/nEdges));
	}
	GL_End();

	SG_GeomDrawEnd(geom);
}

SG_NodeClass sgCircleClass = {
	{
		"SG_Node:SG_Geom:SG_Circle",
		sizeof(SG_Circle),
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
