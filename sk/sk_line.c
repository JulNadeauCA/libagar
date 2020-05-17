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
 * Basic line segment.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Line *
SK_LineNew(void *pnode)
{
	SK_Line *line;

	line = Malloc(sizeof(SK_Line));
	SK_LineInit(line, SK_GenNodeName(SKNODE(pnode)->sk, "Line"));
	SK_NodeAttach(pnode, line);
	return (line);
}

SK_Line *
SK_LineFromValue(void *pnode, M_Line2 L)
{
	SK_Line *Ln;
	M_Vector2 v1, v2;

	Ln = SK_LineNew(pnode);
	Ln->p1 = SK_PointNew(pnode);
	Ln->p2 = SK_PointNew(pnode);
	M_LineToPts2(SK_LineValue(Ln), &v1, &v2);
	SK_Translatev(Ln->p1, &v1);
	SK_Translatev(Ln->p2, &v2);
	return (Ln);
}

M_Line2
SK_LineValue(const SK_Line *Ln)
{
	return M_LineFromPts2(SK_Pos2(Ln->p1), SK_Pos2(Ln->p2));
}

void
SK_LineInit(void *p, Uint name)
{
	SK_Line *line = p;

	SK_NodeInit(line, &skLineOps, name, 0);
	line->width = 1.0;
	line->color = M_ColorRGB(0.0, 0.0, 0.0);
	line->p1 = NULL;
	line->p2 = NULL;
}

int
SK_LineLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Line *line = p;

	line->width = M_ReadReal(buf);
	line->color = M_ReadColor(buf);
	line->p1 = SK_ReadRef(buf, sk, "Point");
	line->p2 = SK_ReadRef(buf, sk, "Point");
	if (line->p1 == NULL || line->p2 == NULL) {
		AG_SetError("Missing endpoint (%s)", AG_GetError());
		return (-1);
	}
	SK_NodeAddReference(line, line->p1);
	SK_NodeAddReference(line, line->p2);
	return (0);
}

int
SK_LineSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Line *line = p;

	M_WriteReal(buf, line->width);
	M_WriteColor(buf, &line->color);
	SK_WriteRef(buf, line->p1);
	SK_WriteRef(buf, line->p2);
	return (0);
}

void
SK_LineDraw(void *p, SK_View *skv)
{
	SK_Line *line = p;
	M_Vector3 v1 = SK_Pos(line->p1);
	M_Vector3 v2 = SK_Pos(line->p2);
	M_Color color = SK_NodeColor(line, &line->color);

	SK_PointDraw(line->p1, skv);
	SK_PointDraw(line->p2, skv);

	GL_Begin(GL_LINES);
	{
		GL_Color3v(&color);
		GL_Vertex3v(&v1);
		GL_Vertex3v(&v2);
	}
	GL_End();
}

void
SK_LineEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Line *line = p;
	const char *unit = skv->sk->uLen->abbr;
//	AG_HSVPal *pal;

	M_NumericalNewReal(box, 0, unit, _("Width: "), &line->width);
//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	M_BindReal(pal, "RGBAv", (void *)&line->color);
}

M_Real
SK_LineProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	SK_Line *line = p;
	M_Vector3 p1 = SK_Pos(line->p1);
	M_Vector3 p2 = SK_Pos(line->p2);
	M_Real mag, u;

	mag = M_VecDistance3(p2, p1);
	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
              ((v->y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);

	*vC = M_VecAdd3(p1, M_VecScale3(M_VecSub3p(&p2,&p1), u));
	if (u < 0.0) {
		*vC = p1;
	} else if (u > 1.0) {
		*vC = p2;
	}
	return M_VecDistance3p(v, vC);
}

int
SK_LineDelete(void *p)
{
	SK_Line *line = p;
	SK *sk = SKNODE(line)->sk;
	int rv;

	SK_NodeDelReference(line, line->p1);
	SK_NodeDelReference(line, line->p2);

	if (SKNODE(line->p1)->nRefs == 0)
		SK_NodeDel(line->p1);
	if (SKNODE(line->p2)->nRefs == 0)
		SK_NodeDel(line->p2);

	rv = SK_NodeDel(line);
	SK_Update(sk);
	return (rv);
}

int
SK_LineMove(void *p, const M_Vector3 *pos, const M_Vector3 *vel)
{
	SK_Line *line = p;

	if (!(SKNODE(line->p1)->flags & (SK_NODE_MOVED|SK_NODE_FIXED))) {
		SK_Translatev(line->p1, vel);
		SKNODE(line->p1)->flags |= SK_NODE_MOVED;
	}
	if (!(SKNODE(line->p2)->flags & (SK_NODE_MOVED|SK_NODE_FIXED))) {
		SK_Translatev(line->p2, vel);
		SKNODE(line->p2)->flags |= SK_NODE_MOVED;
	}
	return (1);
}

/*
 * Lines have two degrees of freedom. Distances and angles correspond to
 * one equation.
 */
SK_Status
SK_LineConstrained(void *p)
{
	SK_Line *L = p;

	SKNODE(L->p1)->flags |= SK_NODE_CHECKED;
	SKNODE(L->p2)->flags |= SK_NODE_CHECKED;

	if (SKNODE(L->p1)->nEdges >= 2 &&
	    SKNODE(L->p2)->nEdges >= 2) {
		if (SKNODE(L)->nEdges > 0) {
			return (SK_OVER_CONSTRAINED);
		} else {
			return (SK_WELL_CONSTRAINED);
		}
	}
	if ((SKNODE(L->p1)->nEdges == 1 && SKNODE(L->p2)->nEdges == 2) ||
	    (SKNODE(L->p1)->nEdges == 2 && SKNODE(L->p2)->nEdges == 1)) {
		if (SKNODE(L)->nEdges == 1) {
			return (SK_WELL_CONSTRAINED);
		} else if (SKNODE(L)->nEdges > 1) {
			return (SK_OVER_CONSTRAINED);
		} else {
			return (SK_UNDER_CONSTRAINED);
		}
	}
	if (SKNODE(L->p1)->nEdges == 1 && SKNODE(L->p2)->nEdges == 1) {
		if (SKNODE(L)->nEdges == 2) {
			return (SK_WELL_CONSTRAINED);
		} else if (SKNODE(L)->nEdges > 2) {
			return (SK_OVER_CONSTRAINED);
		} else {
			return (SK_UNDER_CONSTRAINED);
		}
	}
	return (SK_UNDER_CONSTRAINED);
}

void
SK_LineWidth(SK_Line *line, M_Real size)
{
	line->width = size;
}

void
SK_LineColor(SK_Line *line, M_Color c)
{
	line->color = c;
}

/* Check for shared endpoints between two lines. */
int
SK_LineSharedEndpoint(SK_Line *L1, SK_Line *L2, M_Vector3 *v, M_Vector3 *v1,
    M_Vector3 *v2)
{
	if (L1->p1 == L2->p1) {
		*v = SK_Pos(L1->p1);
		*v1 = SK_Pos(L2->p2);
		*v2 = SK_Pos(L1->p2);
	} else if (L1->p1 == L2->p2) {
		*v = SK_Pos(L1->p1);
		*v1 = SK_Pos(L2->p1);
		*v2 = SK_Pos(L1->p2);
	} else if (L1->p2 == L2->p1) {
		*v = SK_Pos(L1->p2);
		*v1 = SK_Pos(L1->p1);
		*v2 = SK_Pos(L2->p2);
	} else if (L1->p2 == L2->p2) {
		*v = SK_Pos(L1->p2);
		*v1 = SK_Pos(L1->p1);
		*v2 = SK_Pos(L2->p1);
	} else {
		AG_SetError("No shared endpoint");
		return (-1);
	}
	return (0);
}

/*
 * Return the angle that would produce L2 from L1 via counterclockwise
 * rotation.
 */
M_Real
SK_LineLineAngleCCW(SK_Line *L1, SK_Line *L2)
{
	M_Vector3 v, v1, v2;

	if (SK_LineSharedEndpoint(L1, L2, &v, &v1, &v2) == 0) {
		M_Real a1 = Atan2(v2.y-v.y, v2.x-v.x) - M_PI;
		M_Real a2 = Atan2(v1.y-v.y, v1.x-v.x) - M_PI;

		while (a1 > a2) {
			a2 += M_PI*2.0;
		}
		return (a2 - a1);
	}
	return (0.0);
}

SK_NodeOps skLineOps = {
	"Line",
	sizeof(SK_Line),
	0,
	SK_LineInit,
	NULL,			/* destroy */
	SK_LineLoad,
	SK_LineSave,
	SK_LineDraw,
	NULL,			/* redraw */
	SK_LineEdit,
	SK_LineProximity,
	SK_LineDelete,
	SK_LineMove,
	SK_LineConstrained
};

struct sk_line_tool {
	SK_Tool tool;
	SK_Line	 *_Nullable curLine;
	SK_Point *_Nullable curPoint;
};

static void
ToolInit(void *_Nonnull p)
{
	struct sk_line_tool *t = p;

	t->curLine = NULL;
	t->curPoint = NULL;
}

static int
ToolMouseMotion(void *_Nonnull p, M_Vector3 pos, M_Vector3 vel, int btn)
{
	struct sk_line_tool *t = p;
	M_Vector3 vC;
	SK_Point *overPoint;

	overPoint = SK_ViewOverPoint(SKTOOL(t)->skv, &pos, &vC, t->curPoint);
	if (t->curLine != NULL) {
		SK_Identity(t->curPoint);
		SK_Translatev(t->curPoint, (overPoint != NULL) ? &vC : &pos);
	}
	return (0);
}

static int
ToolMouseButtonDown(void *_Nonnull p, M_Vector3 pos, int btn)
{
	struct sk_line_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Point *overPoint;
	SK_Line *line, *lineOther;
	M_Vector3 vC;
	SK_Constraint *ct;

	if (btn != AG_MOUSE_LEFT)
		return (0);

	overPoint = SK_ViewOverPoint(skv, &pos, &vC, t->curPoint);
	if ((line = t->curLine) != NULL) {
		if (overPoint != NULL &&
		    overPoint != line->p2) {

			/* Check that this line does not already exist. */
			SK_FOREACH_NODE(lineOther, sk, sk_line) {
				if ((lineOther->p1 == line->p1 &&
				     lineOther->p2 == overPoint) ||
				    (lineOther->p1 == overPoint &&
				     lineOther->p2 == line->p1))
					return (0);
			}
			if (line->p1 == overPoint)
				return (0);

			/* Remove the temporarily created point. */
			if ((ct = SK_FindConstraint(&sk->ctGraph, SK_INCIDENT,
			    line, line->p2)) != NULL) {
				SK_DelConstraint(&sk->ctGraph, ct);
			}
		    	SK_NodeDelReference(line, line->p2);
			if (SK_NodeDel(line->p2)) {		/* Undo */
				AG_TextMsgFromError();
				SK_AddConstraint(&sk->ctGraph, line, line->p2,
				    SK_INCIDENT);
				return (0);
			}

			/* Assign the selected point. */
			line->p2 = overPoint;
		    	SK_NodeAddReference(line, line->p2);
			SK_AddConstraint(&sk->ctGraph, line, line->p2,
			    SK_INCIDENT);
			SK_UnsuppressConstraints(line->p1);
			SK_UnsuppressConstraints(line->p2);
		}
		t->curLine = NULL;
		t->curPoint = NULL;
		SK_Update(sk);
		return (1);
	}

	line = SK_LineNew(sk->root);
	if (overPoint != NULL) {
		line->p1 = overPoint;
	} else {
		line->p1 = SK_PointNew(sk->root);
		SK_Translatev(line->p1, &pos);
		SK_SuppressConstraints(line->p1);
	}
	SK_NodeAddReference(line, line->p1);
	SK_AddConstraint(&sk->ctGraph, line, line->p1, SK_INCIDENT);

	line->p2 = SK_PointNew(sk->root);
	SK_Translatev(line->p2, &pos);
	SK_SuppressConstraints(line->p2);
	SK_NodeAddReference(line, line->p2);
	SK_AddConstraint(&sk->ctGraph, line, line->p2, SK_INCIDENT);

	t->curLine = line;
	t->curPoint = line->p2;
	
	SK_Update(sk);
	return (1);
}

SK_ToolOps skLineToolOps = {
	N_("Line segment"),
	N_("Insert a line segment into the sketch"),
	NULL,
	sizeof(struct sk_line_tool),
	0,
	ToolInit,
	NULL,			/* destroy */
	NULL,			/* edit */
	ToolMouseMotion,
	ToolMouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
