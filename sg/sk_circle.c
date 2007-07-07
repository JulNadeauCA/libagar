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
 * Basic circle.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"
#include "sk_view.h"
#include "sg_gui.h"

#include <gui/hsvpal.h>

SK_Circle *
SK_CircleNew(void *pnode)
{
	SK_Circle *circle;

	circle = Malloc(sizeof(SK_Circle), M_SG);
	SK_CircleInit(circle, SK_GenName(SKNODE(pnode)->sk));
	SK_NodeAttach(pnode, circle);
	return (circle);
}

void
SK_CircleInit(void *p, Uint32 name)
{
	SK_Circle *circle = p;

	SK_NodeInit(circle, &skCircleOps, name, 0);
	circle->color = SG_ColorRGB(0.0, 0.0, 0.0);
	circle->width = 0.0;
	circle->r = 0.0f;
	circle->p = NULL;
}

int
SK_CircleLoad(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Circle *circle = p;

	circle->color = SG_ReadColor(buf);
	circle->width = SG_ReadReal(buf);
	circle->r = SG_ReadReal(buf);
	circle->p = SK_ReadRef(buf, sk, "Point");
	if (circle->p == NULL) {
		AG_SetError("Missing center point (%s)", AG_GetError());
		return (-1);
	}
	dprintf("%s: width=%f, r=%f, p=%s\n", SK_NodeName(circle),
	    circle->width, circle->r, SK_NodeName(circle->p));
	return (0);
}

int
SK_CircleSave(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Circle *circle = p;

	SG_WriteColor(buf, &circle->color);
	SG_WriteReal(buf, circle->width);
	SG_WriteReal(buf, circle->r);
	SK_WriteRef(buf, circle->p);
	return (0);
}

void
SK_CircleDraw(void *p, SK_View *skv)
{
	SK_Circle *circle = p;
	SG_Vector v = SK_NodeCoords(circle->p);
	SG_Color color = SK_NodeColor(circle, &circle->color);
	SG_Real i, incr;
	
	if (circle->r < skv->wPixel) {
		return;
	}
	incr = (2.0*M_PI)/30.0;

	SG_TranslateVecGL(v);

	SG_Begin(SG_LINE_LOOP);
	SG_Color3v(&color);
	for (i = 0.0; i < M_PI*2.0; i+=incr) {
		glVertex2f(SG_Cos(i)*circle->r,
		           SG_Sin(i)*circle->r);
	}
	SG_End();

	v = SG_VectorMirrorp(&v, 1,1,1);
	SG_TranslateVecGL(v);
}

void
SK_CircleEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Circle *circle = p;
//	AG_HSVPal *pal;

	SG_SpinReal(box, _("Radius: "), &circle->r);
	SG_SpinReal(box, _("Width: "), &circle->width);
//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	SG_WidgetBindReal(pal, "RGBAv", (void *)&circle->color);
}

SG_Real
SK_CircleProximity(void *p, const SG_Vector *v, SG_Vector *vC)
{
	SK_Circle *circle = p;
#if 0
	SG_Vector c = SK_NodeCoords(circle);
	SG_Vector vRel = SG_VectorSubp(v, &c);
	SG_Real theta = SG_Atan2(vRel.y, vRel.x);
	SG_Real rRel = SG_Hypot(vRel.x, vRel.y);

	vC->x = rRel*SG_Sin(theta);
	vC->y = rRel*SG_Cos(theta);
	return (SG_VectorDistancep(v, &c) - circle->r);
#else
	return (HUGE_VAL);
#endif
}

int
SK_CircleDelete(void *p)
{
	SK_Circle *circle = p;
	SK *sk = SKNODE(circle)->sk;

#if 0
	SK_DelConstraint(&sk->ctGraph,
	    SK_FindConstraint(&sk->ctGraph, SK_CONCENTRIC, circle, circle->p));
#endif
	SK_NodeDelReference(circle, circle->p);
	if (SKNODE(circle->p)->nRefs == 0) {
		SK_NodeDel(circle->p);
	}
	return (SK_NodeDel(circle));
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
	SK_CircleEdit,
	SK_CircleProximity,
	SK_CircleDelete
};

#ifdef EDITION

struct sk_circle_tool {
	SK_Tool tool;
	SK_Circle *curCircle;
};

static void
init(void *p)
{
	struct sk_circle_tool *t = p;

	t->curCircle = NULL;
}

static SK_Point *
OverPoint(SK_View *skv, SG_Vector *pos, SG_Vector *vC, void *ignore)
{
	SK *sk = skv->sk;
	SK_Node *node;
	
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
	}
	if ((node = SK_ProximitySearch(sk, "Point", pos, vC, ignore)) != NULL &&
	    SG_VectorDistance2p(pos, vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		return ((SK_Point *)node);
	}
	return (NULL);
}

static int
mousemotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_circle_tool *t = p;
	SG_Vector vCenter, vC;
	SK_Point *overPoint;

	overPoint = OverPoint(SKTOOL(t)->skv, &pos, &vC, NULL);
	if (t->curCircle != NULL) {
		vCenter = SK_NodeCoords(t->curCircle->p);
		t->curCircle->r = SG_VectorDistance(vCenter, pos);
	}
	return (0);
}

static int
mousebuttondown(void *p, SG_Vector pos, int btn)
{
	struct sk_circle_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Circle *circle;
	SK_Point *overPoint;
	SG_Vector vC;

	if (btn != SDL_BUTTON_LEFT)
		return (0);
	
	if (t->curCircle != NULL) {
		t->curCircle = NULL;
		return (1);
	}
	overPoint = OverPoint(skv, &pos, &vC, NULL);
	circle = SK_CircleNew(sk->root);
	if (overPoint != NULL) {
		circle->p = overPoint;
	} else {
		circle->p = SK_PointNew(sk->root);
		SK_Translatev(circle->p, &pos);
	}
#if 0
	SK_AddConstraint(&sk->ctGraph, circle, circle->p, SK_CONCENTRIC);
#endif
	SK_NodeAddReference(circle, circle->p);
	t->curCircle = circle;
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
