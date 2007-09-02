/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Simple arc specified by a radius and two angles.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"

SK_Arc *
SK_ArcNew(void *pnode)
{
	SK_Arc *arc;

	arc = Malloc(sizeof(SK_Arc), M_SG);
	SK_ArcInit(arc, SK_GenNodeName(SKNODE(pnode)->sk, "Arc"));
	SK_NodeAttach(pnode, arc);
	return (arc);
}

void
SK_ArcInit(void *p, Uint32 name)
{
	SK_Arc *arc = p;

	SK_NodeInit(arc, &skArcOps, name, 0);
	arc->color = SG_ColorRGB(0.0, 1.0, 0.0);
	arc->r = 0.0f;
	arc->p = NULL;
	arc->e1 = NULL;
	arc->e2 = NULL;
}

int
SK_ArcLoad(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Arc *arc = p;

	arc->color = SG_ReadColor(buf);
	arc->r = SG_ReadReal(buf);
	arc->p = SK_ReadRef(buf, sk, "Point");
	arc->e1 = SK_ReadRef(buf, sk, "Point");
	arc->e2 = SK_ReadRef(buf, sk, "Point");
	if (arc->p == NULL) {
		AG_SetError("Missing center point (%s)", AG_GetError());
		return (-1);
	}
	if (arc->e1 == NULL || arc->e2 == NULL) {
		AG_SetError("Missing endpoints (%s)", AG_GetError());
		return (-1);
	}
	dprintf("%s: r=%f, p=%s, e1=%s, e2=%s\n", SK_NodeName(arc), arc->r,
	    SK_NodeName(arc->p), SK_NodeName(arc->e1), SK_NodeName(arc->e2));
	return (0);
}

int
SK_ArcSave(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Arc *arc = p;

	SG_WriteColor(buf, &arc->color);
	SG_WriteReal(buf, arc->r);
	SK_WriteRef(buf, arc->p);
	SK_WriteRef(buf, arc->e1);
	SK_WriteRef(buf, arc->e2);
	return (0);
}

void
SK_ArcDraw(void *p, SK_View *skv)
{
	SK_Arc *arc = p;
	SG_Vector v = SK_NodeCoords(arc->p);
	SG_Real i, incr;
	
	if (arc->r < skv->wPixel) {
		return;
	}
	incr = (2*M_PI)/10.0;

	SG_TranslateVecGL(v);

	SG_Begin(SG_LINE_LOOP);
	SG_Color3v(&arc->color);
	for (i = 0.0; i < M_PI*2; i+=incr) {
		glVertex2f(SG_Cos(i)*arc->r,
		           SG_Sin(i)*arc->r);
	}
	SG_End();

	v = SG_VectorMirrorp(&v, 1,1,1);
	SG_TranslateVecGL(v);
}

SG_Real
SK_ArcProximity(void *p, const SG_Vector *v, SG_Vector *vC)
{
	return (HUGE_VAL);
}

void
SK_ArcColor(SK_Arc *arc, SG_Color c)
{
	arc->color = c;
}

SK_NodeOps skArcOps = {
	"Arc",
	sizeof(SK_Arc),
	0,
	SK_ArcInit,
	NULL,		/* destroy */
	SK_ArcLoad,
	SK_ArcSave,
	NULL,		/* draw_relative */
	SK_ArcDraw,
	NULL,		/* redraw */
	NULL,		/* edit */
	SK_ArcProximity,
	NULL,		/* delete */
	NULL,		/* move */
	NULL,		/* constrained */
};

#ifdef EDITION

struct sk_arc_tool {
	SK_Tool tool;
	SK_Arc *cur_arc;
	SK_Point *cur_e1;
};

static void
ToolInit(void *p)
{
	struct sk_arc_tool *t = p;

	t->cur_arc = NULL;
	t->cur_e1 = NULL;
}

static int
ToolMouseMotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_arc_tool *t = p;
	SG_Vector vCenter;

	if (t->cur_arc != NULL) {
		vCenter = SK_NodeCoords(t->cur_arc->p);
		t->cur_arc->r = SG_VectorDistance(vCenter, pos);
	}
	return (0);
}

static int
ToolMouseButtonDown(void *p, SG_Vector pos, int btn)
{
	struct sk_arc_tool *t = p;
	SK *sk = SKTOOL(t)->skv->sk;
	SK_Arc *arc;

	if (btn != SDL_BUTTON_LEFT)
		return (0);

	if (t->cur_arc == NULL) {
		arc = SK_ArcNew(sk->root);
		arc->p = SK_PointNew(sk->root);
		SK_NodeAddReference(arc, arc->p);
		SK_Translatev(arc->p, &pos);
		t->cur_arc = arc;
	} else {
		arc = t->cur_arc;
		if (t->cur_e1 == NULL) {
			arc->e1 = SK_PointNew(sk->root);
			SK_NodeAddReference(arc, arc->e1);
			t->cur_e1 = arc->e1;
		} else {
			arc->e2 = SK_PointNew(sk->root);
			SK_NodeAddReference(arc, arc->e2);
			t->cur_e1 = NULL;
			t->cur_arc = NULL;
		}
	}
	SK_Update(sk);
	return (1);
}

SK_ToolOps skArcToolOps = {
	N_("Arc"),
	N_("Insert an arc into the sketch"),
	VGBEZIER_CURVE_ICON,
	sizeof(struct sk_arc_tool),
	0,
	ToolInit,
	NULL,		/* destroy */
	NULL,		/* edit */
	ToolMouseMotion,
	ToolMouseButtonDown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
