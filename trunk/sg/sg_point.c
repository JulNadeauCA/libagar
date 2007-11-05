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
 * Node object representing a single point in space.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <gui/box.h>

#include "sg.h"
#include "sg_gui.h"

SG_Point *
SG_PointNew(void *pnode, const char *name)
{
	SG_Point *pt;

	pt = Malloc(sizeof(SG_Point));
	SG_PointInit(pt, name);
	SG_NodeAttach(pnode, pt);
	return (pt);
}

void
SG_PointInit(void *p, const char *name)
{
	SG_Point *pt = p;

	SG_NodeInit(pt, name, &sgPointOps, 0);
	pt->size = 5.0;
	pt->color = SG_ColorRGB(1.0, 1.0, 1.0);
}

int
SG_PointLoad(void *p, AG_DataSource *buf)
{
	SG_Point *pt = p;
	
	pt->size = AG_ReadFloat(buf);
	pt->color = SG_ReadColor(buf);
	return (0);
}

int
SG_PointSave(void *p, AG_DataSource *buf)
{
	SG_Point *pt = p;
	
	AG_WriteFloat(buf, pt->size);
	SG_WriteColor(buf, &pt->color);
	return (0);
}

void *
SG_PointEdit(void *p)
{
	SG_Point *pt = p;
	AG_Box *box;

	box = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_EXPAND);
	SG_SpinFloat(box, _("Point size"), &pt->size);
	return (box);
}

void
SG_PointDraw(void *pNode, SG_View *view)
{
	SG_Point *pt = pNode;
	float ptsize_save;
	int lighting_save = glIsEnabled(GL_LIGHTING);

	glDisable(GL_LIGHTING);
	glGetFloatv(GL_POINT_SIZE, &ptsize_save);
	glPointSize(pt->size);
	SG_Begin(GL_POINTS);
	SG_Color4v(&pt->color);
	SG_Vertex2(0.0, 0.0);
	SG_End();
	glPointSize(ptsize_save);
	if (lighting_save) { glEnable(GL_LIGHTING); }
}

void
SG_PointSize(void *pNode, SG_Real size)
{
	SG_Point *pt = pNode;

	pt->size = size;
}

void
SG_PointColor(void *pNode, SG_Color c)
{
	SG_Point *pt = pNode;

	pt->color = c;
}

SG_NodeOps sgPointOps = {
	"Point",
	sizeof(SG_Point),
	0,
	SG_PointInit,
	NULL,		/* destroy */
	SG_PointLoad,
	SG_PointSave,
	NULL,		/* edit */
	NULL,		/* menuInstance */
	NULL,		/* menuClass */
	SG_PointDraw
};

#endif /* HAVE_OPENGL */
