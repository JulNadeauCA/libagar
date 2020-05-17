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
 * Construction geometry: Half-line or line segment.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Line *
SG_LineNew(void *parent, const char *name, const M_Line3 *ml)
{
	SG_Line *ln;

	ln = Malloc(sizeof(SG_Line));
	AG_ObjectInit(ln, &sgLineClass);
	if (name) {
		AG_ObjectSetNameS(ln, name);
	} else {
		OBJECT(ln)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, ln);
	if (ml != NULL) {
		SG_Translatev(ln, ml->p);
		ln->d = ml->d;
		ln->t = ml->t;
	}
	return (ln);
}

static void
Init(void *_Nonnull obj)
{
	SG_Line *ln = obj;

	ln->d = M_VECTOR3(0.0, 0.0, 1.0);
	ln->t = 1.0;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Line *ln = obj;

	ln->d = M_ReadVector3(ds);
	ln->t = M_ReadReal(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Line *ln = obj;

	M_WriteVector3(ds, &ln->d);
	M_WriteReal(ds, ln->t);
	return (0);
}

static void *_Nullable
Edit(void *_Nonnull p, SG_View *_Nullable sgv)
{
	SG_Line *ln = p;
	AG_Mutex *lock = &OBJECT(ln)->lock;
	AG_Box *box;
	AG_Numerical *num;

	box = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_HFILL);
	
	num = AG_NumericalNew(box, 0, NULL, _("Line Length"));
	M_BindRealMp(num, "value", &ln->t, lock);

	return (box);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Line *ln = obj;
	SG_Geom *geom = SGGEOM(ln);

	SG_GeomDrawBegin(geom);

	if (ln->t == M_INFINITY) {
		GL_Begin(GL_LINE_STRIP);
		GL_Color4v(&geom->c);
		GL_Vertex3(0.0, 0.0, 0.0);
		GL_Vertex3v(&ln->d);
		GL_End();
	} else {
		M_Vector3 p2 = ln->d;

		M_VecScale3v(&p2, ln->t);
		GL_Begin(GL_LINES);
		GL_Color4v(&geom->c);
		GL_Vertex3(0.0, 0.0, 0.0);
		GL_Vertex3v(&p2);
		GL_End();
	}

	SG_GeomDrawEnd(geom);
}

SG_NodeClass sgLineClass = {
	{
		"SG_Node:SG_Geom:SG_Line",
		sizeof(SG_Line),
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
	NULL,			/* intersect */
	Edit
};
