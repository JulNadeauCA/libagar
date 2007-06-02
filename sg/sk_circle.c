/*
 * Copyright (c) 2006-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/config/edition.h>
#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>
#include <agar/gui/gui.h>

#include "sk.h"
#include "sg_gui.h"

SK_Circle *
SK_CircleNew(void *pnode)
{
	SK_Circle *circle;

	circle = Malloc(sizeof(SK_Circle), M_SG);
	SK_CircleInit(circle);
	SK_NodeAttach(pnode, circle);
	return (circle);
}

void
SK_CircleInit(void *p)
{
	SK_Circle *circle = p;

	SK_NodeInit(circle, &skCircleOps, 0);
	circle->color = SG_ColorRGB(0.0, 1.0, 0.0);
	circle->width = 0.0;
	circle->r = 0.0f;
	circle->p = NULL;
}

int
SK_CircleLoad(void *p, AG_Netbuf *buf)
{
	SK_Circle *circle = p;

	circle->color = SG_ReadColor(buf);
	circle->width = SG_ReadReal(buf);
	circle->r = SG_ReadReal(buf);
	return (0);
}

int
SK_CircleSave(void *p, AG_Netbuf *buf)
{
	SK_Circle *circle = p;

	SG_WriteColor(buf, &circle->color);
	SG_WriteReal(buf, circle->width);
	SG_WriteReal(buf, circle->r);
	return (0);
}

void
SK_CircleDraw(void *p, SK_View *skv)
{
	SK_Circle *circle = p;
	SG_Vector v = SK_NodeCoords(circle->p);
	SG_Real i, incr;
	
	if (circle->r < skv->wPixel) {
		return;
	}
	incr = (2.0*M_PI)/30.0;

	SG_TranslateVecGL(v);

	SG_Begin(SG_LINE_LOOP);
	SG_Color3v(&circle->color);
	for (i = 0.0; i < M_PI*2.0; i+=incr) {
		glVertex2f(SG_Cos(i)*circle->r,
		           SG_Sin(i)*circle->r);
	}
	SG_End();

	v = SG_VectorMirrorp(&v, 1,1,1);
	SG_TranslateVecGL(v);
}

void
SK_CircleWidth(SK_Circle *circle, SG_Real size)
{
	circle->width = size;
}

void
SK_CircleColor(SK_Circle *circle, SG_Color c)
{
	circle->color = c;
}

SK_NodeOps skCircleOps = {
	"Circle",
	sizeof(SK_Circle),
	0,
	SK_CircleInit,
	NULL,		/* destroy */
	SK_CircleLoad,
	SK_CircleSave,
	NULL,		/* draw_relative */
	SK_CircleDraw,
};

#ifdef EDITION

struct sk_circle_tool {
	SK_Tool tool;
	SK_Circle *cur_circle;
};

static void
init(void *p)
{
	struct sk_circle_tool *t = p;

	t->cur_circle = NULL;
}

static int
mousemotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_circle_tool *t = p;
	SG_Vector vCenter;

	if (t->cur_circle != NULL) {
		vCenter = SK_NodeCoords(t->cur_circle->p);
		t->cur_circle->r = SG_VectorDistance(vCenter, pos);
	}
	return (0);
}

static int
mousebuttondown(void *p, SG_Vector pos, int btn)
{
	struct sk_circle_tool *t = p;
	SK *sk = SKTOOL(t)->skv->sk;
	SK_Circle *circle;

	if (btn != SDL_BUTTON_LEFT)
		return (0);

	if (t->cur_circle != NULL) {
		t->cur_circle = NULL;
		return (1);
	}
	circle = SK_CircleNew(sk->root);
	circle->p = SK_PointNew(sk->root);
	SK_NodeAddReference(circle, circle->p);
	SK_Translatev(circle->p, &pos);
	t->cur_circle = circle;
	return (1);
}

SK_ToolOps skCircleToolOps = {
	N_("Circle"),
	N_("Insert a circle into the sketch"),
	VGCIRCLES_ICON,
	sizeof(struct sk_circle_tool),
	0,
	init,
	NULL,		/* destroy */
	NULL,		/* edit */
	mousemotion,
	mousebuttondown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
