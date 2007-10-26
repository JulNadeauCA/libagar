/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"
#include "sg_gui.h"

SG_Dummy *
SG_DummyNew(void *pnode, const char *name)
{
	SG_Dummy *dum;

	dum = Malloc(sizeof(SG_Dummy), M_SG);
	SG_DummyInit(dum, name);
	SG_NodeAttach(pnode, dum);
	return (dum);
}

void
SG_DummyInit(void *p, const char *name)
{
	SG_Dummy *dum = p;

	SG_NodeInit(dum, name, &sgDummyOps, 0);
}

int
SG_DummyLoad(void *p, AG_DataSource *buf)
{
	SG_Dummy *dum = p;

	dum->foo = SG_ReadReal(buf);
	return (0);
}

int
SG_DummySave(void *p, AG_DataSource *buf)
{
	SG_Dummy *dum = p;

	SG_WriteReal(buf, dum->foo);
	return (0);
}

void
SG_DummyDraw(void *p, SG_View *view)
{
	glBegin(GL_POINTS);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
}

void
SG_DummyEdit(void *p, AG_Widget *box, SG_View *sgv)
{
	SG_Dummy *dum = p;

	SG_SpinFloat(box, _("Foo"), &dum->foo);
}

SG_NodeOps sgDummyOps = {
	"Dummy",
	sizeof(SG_Dummy),
	0,
	SG_DummyInit,
	NULL,		/* destroy */
	SG_DummyLoad,
	SG_DummySave,
	SG_DummyEdit,
	NULL,		/* menuInstance */
	NULL,		/* menuClass */
	SG_DummyDraw,
};

#endif /* HAVE_OPENGL */
