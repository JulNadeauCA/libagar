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
 * Dummy sketch object.
 */ 

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Dummy *
SK_DummyNew(void *pnode)
{
	SK_Dummy *dum;

	dum = Malloc(sizeof(SK_Dummy));
	SK_DummyInit(dum, SK_GenNodeName(SKNODE(pnode)->sk, "Dummy"));
	SK_NodeAttach(pnode, dum);
	return (dum);
}

void
SK_DummyInit(void *p, Uint name)
{
	SK_Dummy *dum = p;

	SK_NodeInit(dum, &skDummyOps, name, 0);
}

int
SK_DummyLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Dummy *dum = p;

	dum->foo = M_ReadReal(buf);
	return (0);
}

int
SK_DummySave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Dummy *dum = p;

	M_WriteReal(buf, dum->foo);
	return (0);
}

void
SK_DummyDraw(void *p, SK_View *view)
{
	glBegin(GL_POINTS);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
}

SK_NodeOps skDummyOps = {
	"Dummy",
	sizeof(SK_Dummy),
	0,
	SK_DummyInit,
	NULL,		/* destroy */
	SK_DummyLoad,
	SK_DummySave,
	SK_DummyDraw,
	NULL,		/* redraw */
	NULL,		/* edit */
	NULL,		/* proximity */
	NULL,		/* delete */
	NULL,		/* constrained */
};
