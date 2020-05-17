/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Construction geometry: Single point.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Point *
SG_PointNew(void *parent, const char *name, const M_Vector3 *v)
{
	SG_Point *pt;

	pt = Malloc(sizeof(SG_Point));
	AG_ObjectInit(pt, &sgPointClass);
	if (name) {
		AG_ObjectSetNameS(pt, name);
	} else {
		OBJECT(pt)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, pt);
	if (v != NULL) {
		SG_Translatev(pt, *v);
	}
	return (pt);
}

static void
Init(void *_Nonnull obj)
{
	SG_Point *pt = obj;

	pt->size = 5.0;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_Point *pt = obj;
	
	pt->size = AG_ReadFloat(buf);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SG_Point *pt = obj;
	
	AG_WriteFloat(buf, pt->size);
	return (0);
}

static void *_Nullable
Edit(void *_Nonnull p, SG_View *_Nullable sgv)
{
	SG_Point *pt = p;
	AG_Box *box;
	AG_Numerical *num;

	box = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_HFILL);
	num = AG_NumericalNew(box, 0, "px", _("Point size"));
	AG_BindFloatMp(num, "value", &pt->size, &OBJECT(pt)->lock);
	return (box);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	SG_Point *pt = obj;
	SG_Geom *geom = SGGEOM(pt);
	float ptSaved;

	if (pt->size == 0)
		return;

	GL_PushAttrib(GL_LIGHTING_BIT);
	GL_Disable(GL_LIGHTING);

	GL_GetFloatv(GL_POINT_SIZE, &ptSaved);
	GL_PointSize(pt->size);

	GL_Begin(GL_POINTS);
	{
		GL_Color4v(&geom->c);
		GL_Vertex2(0.0, 0.0);
	}
	GL_End();

	GL_PointSize(ptSaved);

	GL_PopAttrib();
}

/* Set a point size for rendering. */
void
SG_PointSize(void *obj, M_Real size)
{
	SG_Point *pt = obj;

	AG_ObjectLock(pt);
	pt->size = (float)size;
	AG_ObjectUnlock(pt);
}

SG_NodeClass sgPointClass = {
	{
		"SG_Node:SG_Geom:SG_Point",
		sizeof(SG_Point),
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
