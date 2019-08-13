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
 * Box-shaped polyhedral object.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/* Create a new Box node. */
SG_Polybox *
SG_PolyboxNew(void *parent, const char *name)
{
	SG_Polybox *box;

	box = Malloc(sizeof(SG_Polybox));
	AG_ObjectInit(box, &sgPolyboxClass);
	if (name) {
		AG_ObjectSetNameS(box, name);
	} else {
		OBJECT(box)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, box);
	return (box);
}

static void
Init(void *_Nonnull obj)
{
	SG_Polybox *box = obj;

	SG_EdgeRehash(box, 8);

	SG_VertexNew(box, M_VECTOR3(-1.0, -1.0, -1.0));
	SG_VertexNew(box, M_VECTOR3(+1.0, -1.0, -1.0));
	SG_VertexNew(box, M_VECTOR3(+1.0, -1.0, +1.0));
	SG_VertexNew(box, M_VECTOR3(-1.0, -1.0, +1.0));
	
	SG_VertexNew(box, M_VECTOR3(-1.0, +1.0, +1.0));
	SG_VertexNew(box, M_VECTOR3(+1.0, +1.0, +1.0));
	SG_VertexNew(box, M_VECTOR3(+1.0, +1.0, -1.0));
	SG_VertexNew(box, M_VECTOR3(-1.0, +1.0, -1.0));

	OBJ_ST(box,1) = M_VECTOR2(0.25, 0.50);
	OBJ_ST(box,2) = M_VECTOR2(0.50, 0.50);
	OBJ_ST(box,3) = M_VECTOR2(0.50, 0.25);
	OBJ_ST(box,4) = M_VECTOR2(0.25, 0.25);
	OBJ_ST(box,5) = M_VECTOR2(0.25, 0.00); /* [0,0.25]; [1,0.25] */
	OBJ_ST(box,6) = M_VECTOR2(0.75, 0.25);
	OBJ_ST(box,7) = M_VECTOR2(0.75, 0.50);
	OBJ_ST(box,8) = M_VECTOR2(0.00, 0.50); /* [0.25,0]; [1,0.50] */

	SG_FacetFromQuad4(box, 1,2,3,4);
	SG_FacetFromQuad4(box, 2,7,6,3);
	SG_FacetFromQuad4(box, 1,4,5,8);
	SG_FacetFromQuad4(box, 1,8,7,2);
	SG_FacetFromQuad4(box, 4,3,6,5);
	SG_FacetFromQuad4(box, 5,6,7,8);

	SG_ObjectNormalize(box);
}

SG_NodeClass sgPolyboxClass = {
	{
		"SG_Node:SG_Object:SG_Polybox",
		sizeof(SG_Polybox),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		SG_NodeEdit
	},
	NULL,			/* menuNode */
	NULL,			/* menuClass */
	NULL,			/* draw */
	NULL,			/* intersect */
	NULL			/* edit */
};
