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
 * Basic circle.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Circle *
SK_CircleNew(void *pnode)
{
	SK_Circle *circle;

	circle = Malloc(sizeof(SK_Circle));
	SK_CircleInit(circle, SK_GenNodeName(SKNODE(pnode)->sk, "Circle"));
	SK_NodeAttach(pnode, circle);
	return (circle);
}

SK_Circle *
SK_CircleFromValue(void *pnode, M_Circle2 C)
{
	SK_Circle *Cn;

	Cn = SK_CircleNew(pnode);
	Cn->r = C.r;
	Cn->p = SK_PointNew(pnode);
	SK_Translatev(Cn->p, &C.p);
	return (Cn);
}

M_Circle2
SK_CircleValue(SK_Circle *Cn)
{
	M_Circle2 C;

	C.p = SK_Pos2(Cn->p);
	C.r = Cn->r;
	return (C);
}

void
SK_CircleInit(void *p, Uint name)
{
	SK_Circle *circle = p;

	SK_NodeInit(circle, &skCircleOps, name, 0);
	circle->color = M_ColorBlack();
	circle->width = 0.0;
	circle->r = 0.0f;
	circle->p = NULL;
}

int
SK_CircleLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Circle *circle = p;

	circle->color = M_ReadColor(buf);
	circle->width = M_ReadReal(buf);
	circle->r = M_ReadReal(buf);
	circle->p = SK_ReadRef(buf, sk, "Point");
	if (circle->p == NULL) {
		AG_SetError("Missing center point (%s)", AG_GetError());
		return (-1);
	}
	SK_NodeAddReference(circle, circle->p);
	return (0);
}

int
SK_CircleSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Circle *circle = p;

	M_WriteColor(buf, &circle->color);
	M_WriteReal(buf, circle->width);
	M_WriteReal(buf, circle->r);
	SK_WriteRef(buf, circle->p);
	return (0);
}

void
SK_CircleDraw(void *p, SK_View *skv)
{
	SK_Circle *circle = p;
	M_Vector3 v = SK_Pos(circle->p);
	M_Color color = SK_NodeColor(circle, &circle->color);
	M_Real i, incr;

	SK_PointDraw(circle->p, skv);

	if (circle->r < skv->wPixel)
		return;

	/* XXX */
	incr = (2.0*M_PI)/30.0;

	GL_Translatev(&v);
	GL_Begin(GL_LINE_LOOP);
	GL_Color3v(&color);
	for (i = 0.0; i < M_PI*2.0; i+=incr) {
		GL_Vertex2(Cos(i)*circle->r,
		           Sin(i)*circle->r);
	}
	GL_End();
	GL_Translate(M_VecFlip3(v));
}

void
SK_CircleEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Circle *circle = p;
	const char *unit = skv->sk->uLen->abbr;
//	AG_HSVPal *pal;

	M_NumericalNewReal(box, 0, unit, _("Radius: "), &circle->r);
	M_NumericalNewReal(box, 0, unit, _("Width: "), &circle->width);
//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	M_BindReal(pal, "RGBAv", (void *)&circle->color);
}

M_Real
SK_CircleProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	SK_Circle *circle = p;
	M_Vector3 c = SK_Pos(circle->p);
	M_Vector3 vRel = M_VecSub3p(v, &c);
	M_Real theta = Atan2(vRel.y, vRel.x);

	vC->x = c.x + circle->r*Cos(theta);
	vC->y = c.y + circle->r*Sin(theta);
	vC->z = 0.0;
	return M_VecDistance3p(v, vC);
}

int
SK_CircleDelete(void *p)
{
	SK_Circle *circle = p;
	SK *sk = SKNODE(circle)->sk;
	int rv;

	SK_NodeDelReference(circle, circle->p);
	if (SKNODE(circle->p)->nRefs == 0) {
		SK_NodeDel(circle->p);
	}
	rv = SK_NodeDel(circle);
	SK_Update(sk);
	return (rv);
}

int
SK_CircleMove(void *p, const M_Vector3 *pos, const M_Vector3 *vel)
{
	SK_Circle *c = p;

	if (!(SKNODE(c->p)->flags & SK_NODE_MOVED)) {
		SK_Translatev(c->p, vel);
		SKNODE(c->p)->flags |= SK_NODE_MOVED;
	}
	return (1);
}

/*
 * Circles in 2D require three constraints, two for the center point
 * and one for the radius.
 */
SK_Status
SK_CircleConstrained(void *p)
{
	SK_Circle *C = p;

	SKNODE(C->p)->flags |= SK_NODE_CHECKED;

	if (SKNODE(C->p)->nEdges == 2) {
		if (SKNODE(C)->nEdges == 1) {
			return (SK_WELL_CONSTRAINED);
		} else if (SKNODE(C)->nEdges < 1) {
			return (SK_UNDER_CONSTRAINED);
		} else if (SKNODE(C)->nEdges > 1) {
			return (SK_OVER_CONSTRAINED);
		}
	} else if (SKNODE(C->p)->nEdges < 2) {
		return (SK_UNDER_CONSTRAINED);
	}
	return (SK_OVER_CONSTRAINED);
}

void
SK_CircleColor(SK_Circle *circle, M_Color c)
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
	SK_CircleDraw,
	NULL,		/* redraw */
	SK_CircleEdit,
	SK_CircleProximity,
	SK_CircleDelete,
	SK_CircleMove,
	SK_CircleConstrained
};

struct sk_circle_tool {
	SK_Tool tool;
	SK_Circle *curCircle;
};

static void
ToolInit(void *_Nonnull p)
{
	struct sk_circle_tool *t = p;

	t->curCircle = NULL;
}

static int
ToolMouseMotion(void *_Nonnull p, M_Vector3 pos, M_Vector3 vel, int btn)
{
	struct sk_circle_tool *t = p;
	M_Vector3 vCenter, vC;
	/* SK_Point *overPoint; */

	(void)SK_ViewOverPoint(SKTOOL(t)->skv, &pos, &vC, NULL);
	if (t->curCircle != NULL) {
		vCenter = SK_Pos(t->curCircle->p);
		t->curCircle->r = M_VecDistance3(vCenter, pos);
	}
	return (0);
}

static int
ToolMouseButtonDown(void *_Nonnull p, M_Vector3 pos, int btn)
{
	struct sk_circle_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Circle *circle;
	SK_Point *overPoint;
	M_Vector3 vC;

	if (btn != AG_MOUSE_LEFT)
		return (0);
	
	if (t->curCircle != NULL) {
		t->curCircle = NULL;
		return (1);
	}
	overPoint = SK_ViewOverPoint(skv, &pos, &vC, NULL);
	circle = SK_CircleNew(sk->root);
	if (overPoint != NULL) {
		circle->p = overPoint;
	} else {
		circle->p = SK_PointNew(sk->root);
		SK_Translatev(circle->p, &pos);
	}
	SK_NodeAddReference(circle, circle->p);
	t->curCircle = circle;
	SK_Update(sk);
	return (1);
}

SK_ToolOps skCircleToolOps = {
	N_("Circle"),
	N_("Insert a circle into the sketch"),
	NULL,
	sizeof(struct sk_circle_tool),
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
