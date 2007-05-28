/*
 * Copyright (c) 2006-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
 
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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>
#include <agar/gui/gui.h>

#include "sk.h"
#include "sg_gui.h"

SK_Point *
SK_PointNew(void *pnode)
{
	SK_Point *pt;

	pt = Malloc(sizeof(SK_Point), M_SG);
	SK_PointInit(pt);
	SK_NodeAttach(pnode, pt);
	return (pt);
}

void
SK_PointInit(void *p)
{
	SK_Point *pt = p;

	SK_NodeInit(pt, &skPointOps, 0);
	pt->size = 3.0;
	pt->color = SG_ColorRGB(0.0, 1.0, 0.0);
}

int
SK_PointLoad(void *p, AG_Netbuf *buf)
{
	SK_Point *pt = p;

	pt->size = SG_ReadReal(buf);
	pt->color = SG_ReadColor(buf);
	return (0);
}

int
SK_PointSave(void *p, AG_Netbuf *buf)
{
	SK_Point *pt = p;

	SG_WriteReal(buf, pt->size);
	SG_WriteColor(buf, &pt->color);
	return (0);
}

void
SK_PointDraw(void *p, SK_View *skv)
{
	SK_Point *pt = p;

	glBegin(GL_LINES);
	glColor3f(pt->color.r, pt->color.g, pt->color.b);
	glVertex2f(0.0, 0.0);	glVertex2f(-pt->size*skv->wPixel, 0.0);
	glVertex2f(0.0, 0.0);	glVertex2f(+pt->size*skv->wPixel, 0.0);
	glVertex2f(0.0, 0.0);	glVertex2f(0.0, -pt->size*skv->hPixel);
	glVertex2f(0.0, 0.0);	glVertex2f(0.0, +pt->size*skv->hPixel);
	glEnd();
}

void
SK_PointSize(SK_Point *pt, SG_Real size)
{
	pt->size = size;
}

void
SK_PointColor(SK_Point *pt, SG_Color c)
{
	pt->color = c;
}

SK_NodeOps skPointOps = {
	"Point",
	sizeof(SK_Point),
	0,
	SK_PointInit,
	NULL,		/* destroy */
	SK_PointLoad,
	SK_PointSave,
	SK_PointDraw,
	NULL		/* draw_absolute */
};

#ifdef EDITION

static int
mousebuttondown(void *pTool, SG_Vector pos, int btn)
{
	SK_Tool *tool = pTool;
	SK *sk = tool->skv->sk;
	SK_Point *pt;

	pt = Malloc(sizeof(SK_Point), M_OBJECT);
	SK_PointInit(pt);
	SK_NodeAttach(sk->root, pt);
	SG_MatrixTranslate2(&SKNODE(pt)->T, pos.x, pos.y);
	return (0);
}

SK_ToolOps skPointToolOps = {
	N_("Point"),
	N_("Insert a point into the sketch"),
	VGPOINTS_ICON,
	sizeof(SK_Tool),
	0,
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* edit */
	NULL,		/* motion */
	mousebuttondown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
