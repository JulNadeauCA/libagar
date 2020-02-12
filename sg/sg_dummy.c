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
 * Sample node object.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/*
 * Constructor routine. This is optional and only useful if this API is going
 * to be used directly.
 */
SG_Dummy *
SG_DummyNew(void *parent, const char *name)
{
	SG_Dummy *dum;

	dum = Malloc(sizeof(SG_Dummy));
	AG_ObjectInit(dum, &sgDummyClass);
	if (name) {
		AG_ObjectSetNameS(dum, name);
	} else {
		OBJECT(dum)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, dum);
	return (dum);
}

static void
Init(void *_Nonnull obj)
{
	SG_Dummy *dum = obj;

	dum->foo = 1.234;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Dummy *dum = obj;

	dum->foo = M_ReadReal(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Dummy *dum = obj;

	M_WriteReal(ds, dum->foo);
	return (0);
}

/* Rendering routine. This draws our object at the proper place in the graph. */
static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glVertex3f(1.0, 1.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glEnd();
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Dummy *dum = obj;
	AG_Mutex *lock = &OBJECT(dum)->lock;
	AG_Box *box;
	AG_Numerical *num;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);
	num = AG_NumericalNew(box, 0, NULL, _("Foo"));
	M_BindRealMp(num, "value", &dum->foo, lock);

	return (box);
}

SG_NodeClass sgDummyClass = {
	{
		"SG_Node:SG_Dummy",
		sizeof(SG_Dummy),
		{ 0,0 },
		Init,
		NULL,	/* reset */
		NULL,	/* destroy */
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
